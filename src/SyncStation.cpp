#pragma once
#include "stdafx.h"
#include "SyncStation.h"

SyncStation::RWHandle::RWHandle(LONG read_job_init_count, LONG read_job_max_count)
	: mutex_(make_wrapper_hub<SyncMutex>()), semaphore_(make_wrapper_hub<SyncSemaphore>(read_job_init_count, read_job_max_count))
{}

decltype(auto) SyncStation::RWHandle::state()
{
	DWORD ret = WaitForSingleObject(mutex_.get(), WAIT_TIME_ZERO);
	if (ret == WAIT_OBJECT_0)
		return HANDLE_STATE::WRITE_LOCK;

	ret = WaitForSingleObject(semaphore_.get(), WAIT_TIME_ZERO);
	if (ret == WAIT_OBJECT_0)
		return HANDLE_STATE::READ_LOCK;

	return HANDLE_STATE::IDLE;
}

bool SyncStation::RecordHandle(TypeID tid, LONG read_job_init_count, LONG read_job_max_count)
{
	//std::unique_lock<std::mutex>(mtx_);
	if (handle_by_type_.find(tid) == handle_by_type_.end())
		return false;

	assert(handle_by_type_.size() == write_handles_.size() && write_handles_.size() == read_handles_.size());

	auto ret = handle_by_type_.emplace(tid, std::forward<RWHandle>(RWHandle(read_job_init_count, read_job_max_count)));
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

SyncStation::HANDLE_STATE SyncStation::handle_state(TypeID tid)
{
	std::unique_lock<std::mutex>(distribute_mtx_);

	auto itor = handle_by_type_.find(tid);
	if (itor == handle_by_type_.end())
	{
		assert(false);
		return HANDLE_STATE::IDLE;
	}

	return itor->second.state();
}

bool SyncStation::IsRecordType(TypeID tid)
{
	auto itor = handle_by_type_.find(tid);
	return false;
}
