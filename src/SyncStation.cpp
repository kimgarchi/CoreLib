#pragma once
#include "stdafx.h"
#include "SyncStation.h"
#include "Job.h"

SyncStation::RWHandle::RWHandle(WORD idx)
	: idx_(idx), 
	mutex_(make_wrapper_hub<sync::Mutex>()), 
	semaphore_(make_wrapper_hub<sync::Semaphore>())
{}

decltype(auto) SyncStation::RWHandle::state()
{
	DWORD ret = WaitForSingleObject(mutex_.get(), 0);
	if (ret == WAIT_OBJECT_0)
		return HANDLE_STATE::WRITE_LOCK;

	ret = WaitForSingleObject(semaphore_.get(), 0);
	if (ret == WAIT_OBJECT_0)
		return HANDLE_STATE::READ_LOCK;

	return HANDLE_STATE::IDLE;
}

bool SyncStation::RegistReadJob(HarvestTypes types, JobUnit job_unit)
{

	return false;
}

bool SyncStation::RegistWriteJob(HarvestTypes types, JobUnit job_unit)
{
	// ...
	return false;
}

bool SyncStation::RecordHandle(TypeID tid)
{
	/*
	//std::unique_lock<std::mutex>(mtx_);
	if (handle_by_type_.find(tid) == handle_by_type_.end())
		return false;

	auto idx = handle_by_type_.size();
	assert(handle_by_type_.size() == write_handles_.size() && write_handles_.size() == read_handles_.size());

	auto ret = handle_by_type_.emplace(tid, idx);
	auto push_result = ret.second;
	auto rw_handle = ret.first->second;

	if (push_result == false)
	{
		assert(false);
		return false;
	}

	write_handles_.emplace_back(rw_handle.WriteHandle());
	read_handles_.emplace_back(rw_handle.Readhandle());
	*/
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
