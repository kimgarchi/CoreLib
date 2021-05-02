#pragma once
#include "stdafx.h"
#include "SyncStation.h"
#include "ThreadManager.h"

SyncStation::SyncStation()
	: mutex_hub_(make_wrapper_hub<SyncMutex>()), event_hub_(make_wrapper_hub<SyncEvent>())
{
}

SyncStation::~SyncStation()
{
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