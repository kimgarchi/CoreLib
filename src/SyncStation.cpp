#pragma once
#include "stdafx.h"
#include "SyncStation.h"
#include "ThreadManager.h"

SyncStation::RWHandle::RWHandle(LONG read_job_max_count)
	: mutex_(make_wrapper_hub<SyncMutex>()), semaphore_(make_wrapper_hub<SyncSemaphore>(read_job_max_count))
{}

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

bool SyncStation::RecordHandle(TypeID tid, LONG read_job_max_count)
{
	if (handle_by_type_.find(tid) != handle_by_type_.end())
	{
		assert(false);
		return true;
	}
	
	auto array_idx = handle_by_type_.size() + 1;
	auto ret = handle_by_type_.emplace(tid, make_wrapper_hub<RWHandle>(read_job_max_count));
	
	if (ret.second == false)
	{
		assert(false);
		return false;
	}

	return true;
}

bool SyncStation::IsRecordType(TypeID tid)
{
	auto itor = handle_by_type_.find(tid);
	return false;
}

bool SyncStation::RegistJob(TypeIds tids, JobBaseHub job)
{
	return false;
}

SyncStation::DistributeJob::DistributeJob(HandleByType& handle_by_type, ReserveJobQue& reserve_job_que, SyncMutexNode mutex_node, SyncEventNode event_node)
	: handle_by_type_(handle_by_type), reserve_job_que_(reserve_job_que), mutex_node_(mutex_node), event_node_(event_node)
{
}

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

		if (wait_job->Aquire() == false)
		{
			wait_job_priority_que_.push(wait_job);
			continue;
		}
		
	}

	assert(event_node_->Release());

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
	HANDLE* phandle = nullptr;
	
	switch (type_)
	{
	case JOB_TYPE::READ:
		handle_count = static_cast<DWORD>(read_handles_.size());
		phandle = read_handles_.data();
		break;
	case JOB_TYPE::WRITE:
		handle_count = static_cast<DWORD>(write_handles_.size());
		phandle = write_handles_.data();
		break;
	}

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
