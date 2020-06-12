#pragma once
#include "stdafx.h"
#include "SyncStation.h"
#include "ThreadManager.h"

SyncStation::SyncStation()
	: mutex_hub_(make_wrapper_hub<SyncMutex>()), event_hub_(make_wrapper_hub<SyncEvent>()), distribute_task_id_(INVALID_ALLOC_ID)
{
	const size_t single_count = 1;
	distribute_task_id_ = ThreadManager::GetInstance().AttachTask<DistributeJob>(single_count, std::ref(handle_by_type_), std::ref(reserve_job_que_), mutex_hub_.make_node(), event_hub_.make_node());
	if (distribute_task_id_ == INVALID_ALLOC_ID)
	{
		assert(false);
		throw std::runtime_error("sync station thread bind failed");
	}
}

SyncStation::~SyncStation()
{
	assert(ThreadManager::GetInstance().DeattachTask(distribute_task_id_));
}

HANDLE SyncStation::RecordHandle(TypeID tid, LONG read_job_max_count)
{
	if (handle_by_type_.find(tid) != handle_by_type_.end())
	{
		assert(false);
		return INVALID_HANDLE_VALUE;
	}
	
	auto array_idx = handle_by_type_.size() + 1;
	auto sema_hub = make_wrapper_hub<SyncSemaphore>(read_job_max_count);
	auto ret = handle_by_type_.emplace(tid, sema_hub);
	
	if (ret.second == false)
	{
		assert(false);
		return INVALID_HANDLE_VALUE;
	}

	return sema_hub->handle();
}

SyncStation::DistributeJob::DistributeJob(HandleByType& handle_by_type, ReserveJobQue& reserve_job_que, SyncMutexNode mutex_node, SyncEventNode event_node)
	: handle_by_type_(handle_by_type), reserve_job_que_(reserve_job_que), mutex_node_(mutex_node), event_node_(event_node)
{}

bool SyncStation::DistributeJob::Work()
{
	if (event_node_->Lock() == false)
	{
		assert(false);
		return false;
	}

	{
		SingleLock single_lock(mutex_node_);
		while (reserve_job_que_.empty() == false)
		{
			auto reserve_job = reserve_job_que_.front();

			for (auto itor = reserve_job->tids_.cbegin(); itor != reserve_job->tids_.cend(); ++itor)
			{
				if (handle_by_type_.find(*itor) != handle_by_type_.cend())
					continue;

				assert(SyncStation::GetInstance().RecordHandle(*itor));
			}

			wait_job_priority_que_.emplace(reserve_job);
			reserve_job_que_.pop();
		}
	}

	for (size_t i = 0; i < wait_job_priority_que_.size(); ++i)
	{
		ReservePackageHub wait_job = wait_job_priority_que_.top();
		wait_job_priority_que_.pop();

		if (Prepare(std::ref(wait_job)) == false)
			throw std::runtime_error("reserve package prepare failed");
		
		if (wait_job->Aquire() == false)
		{
			wait_job_priority_que_.push(wait_job);
			continue;
		}
	}

	assert(event_node_->Release());

	return true;
}

bool SyncStation::DistributeJob::Prepare(ReservePackageHub& package)
{
	auto& tids = package->tids_;
	package->handles_.resize(tids.size(), { INVALID_HANDLE_VALUE, } );

	size_t idx = 0;
	for (auto itor = tids.cbegin(); itor != tids.cend(); ++itor, ++idx)
	{
		const auto& tid = *itor;
		HANDLE handle = INVALID_HANDLE_VALUE;
		
		auto type_itor = handle_by_type_.find(tid);
		if (type_itor == handle_by_type_.end())
			handle = SyncStation::GetInstance().RecordHandle(tid);
		
		if (handle == INVALID_HANDLE_VALUE)
		{
			assert(false);
			return false;
		}

		package->handles_.at(idx) = handle;
	}

	return true;
}

SyncStation::ReservePackage::ReservePackage(JOB_TYPE&& type, TypeIds&& tids, JobBaseNode&& job_node)
	: type_(type), job_node_(job_node), try_count_(0)
{		
}

SyncStation::ReservePackage::~ReservePackage()
{
	// Release
}

bool SyncStation::ReservePackage::Aquire()
{
	increase_try_count();

	DWORD handle_count = 0;
	PHANDLE phandle = nullptr;
	
	auto ret = WaitForMultipleObjects(handle_count, phandle, true, WAIT_TIME_ZERO);
	
	switch (ret)
	{
	case WAIT_TIMEOUT:
	case WAIT_FAILED:
		assert(false);
		return false;
	}

	if (ret != WAIT_OBJECT_0 + (handle_count))
	{
		/*
		WAIT_OBJECT_ABANDONED
		handle invalid...
		*/	
		assert(false);
		// temp
		return false;
	}

	return true;
}