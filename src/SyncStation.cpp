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

bool SyncStation::RegistReadJob(JobUnit job_unit)
{
	// ... 
	return false;
}

bool SyncStation::RegistWriteJob(JobUnit job_unit)
{
	// ...
	return false;
}
