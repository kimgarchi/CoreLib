#pragma once
#include "stdafx.h"
#include "SyncStation.h"
#include "ThreadManager.h"

SyncStation::RWHandle::RWHandle(size_t array_idx, LONG read_job_max_count)
	: array_idx_(array_idx), mutex_(make_wrapper_hub<SyncMutex>()), semaphore_(make_wrapper_hub<SyncSemaphore>(read_job_max_count))
{}

decltype(auto) SyncStation::RWHandle::state()
{
	auto state = mutex_->state();
	if (state == SYNC_STATE::FULL_LOCK)
		return state;

	return semaphore_->state();
}

SyncStation::SyncStation()
	: mutex_hub_(make_wrapper_hub<SyncMutex>()), task_id_(INVALID_ALLOC_ID)
{
	const size_t single_count = 1;
	task_id_ = ThreadManager::GetInstance().AttachTask<DistributeJob>(single_count, *this);
	if (task_id_ == INVALID_ALLOC_ID)
	{
		assert(false);
		throw std::runtime_error("sync station thread bind failed");
	}
}

SyncStation::~SyncStation()
{
	assert(ThreadManager::GetInstance().DeattachTask(task_id_));
}

bool SyncStation::RecordHandle(TypeID tid, LONG read_job_max_count)
{
	if (handle_by_type_.find(tid) != handle_by_type_.end())
	{
		assert(false);
		return true;
	}
	
	assert(handle_by_type_.size() == write_handles_.size() && handle_by_type_.size() == read_handles_.size());

	auto array_idx = handle_by_type_.size() + 1;
	auto ret = handle_by_type_.emplace(tid, std::forward<RWHandle>(RWHandle(array_idx, read_job_max_count)));
	auto push_result = ret.second;
	auto rw_handle = ret.first->second;

	if (push_result == false)
	{
		assert(false);
		return false;
	}

	write_handles_.emplace_back(rw_handle.WriteHandle());
	read_handles_.emplace_back(rw_handle.Readhandle());

	return true;
}

SYNC_STATE SyncStation::handle_state(TypeID tid)
{
	std::unique_lock<std::mutex>(distribute_mtx_);

	auto itor = handle_by_type_.find(tid);
	if (itor == handle_by_type_.end())
	{
		assert(false);
		return SYNC_STATE::FULL_LOCK;
	}

	return itor->second.state();
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

SyncStation::DistributeJob::DistributeJob(SyncStation& sync_station)
	: sync_station_(sync_station)
{
}

bool SyncStation::DistributeJob::Work()
{

	return false;
}

size_t SyncStation::DistributeJob::ShiftQue()
{
	auto mutex_node = sync_station_.mutex_node();
	SingleLock single_lock(mutex_node);

	auto& reserve_que = sync_station_.reserve_job_que();
	auto& wait_que = sync_station_.wait_job_que();

	auto shift_size = reserve_que.size();

	while (reserve_que.empty() == false)
	{

	}

	return shift_size;
}
