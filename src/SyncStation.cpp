#pragma once
#include "stdafx.h"
#include "SyncStation.h"
#include "ThreadManager.h"

SyncStation::SyncStation()
	//: mutex_ptr_(allocate_shared<SyncMutex>()), event_ptr_(allocate_shared<SyncEvent>())
	: mutex_ptr_(std::make_shared<SyncMutex>()), event_ptr_(std::make_shared<SyncEvent>())
{
	assert(mutex_ptr_);
	assert(event_ptr_);
}

SyncStation::~SyncStation()
{
}

HANDLE SyncStation::RecordHandle(std::type_index type_idx, LONG read_job_max_count)
{
	if (handle_by_type_.find(type_idx) != handle_by_type_.end())
	{
		assert(false);
		return INVALID_HANDLE_VALUE;
	}
	
	auto array_idx = handle_by_type_.size() + 1;
	auto semaphore_ptr = std::make_shared<SyncSemaphore>(read_job_max_count); //allocate_shared<SyncSemaphore>(read_job_max_count);
	auto ret = handle_by_type_.emplace(type_idx, semaphore_ptr);
	
	if (ret.second == false)
	{
		assert(false);
		return INVALID_HANDLE_VALUE;
	}

	return semaphore_ptr->handle();
}