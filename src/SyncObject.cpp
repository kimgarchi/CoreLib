#pragma once
#include "stdafx.h"
#include "SyncObject.h"

SyncHandle::SyncHandle(HANDLE handle)
	: handle_(handle)
{
	assert(handle != INVALID_HANDLE_VALUE);
}

SyncHandle::~SyncHandle()
{
	CloseHandle(handle_);
}

SyncMutex::SyncMutex()
	: SyncHandle(::CreateMutex(nullptr, false, nullptr))
{}

SyncMutex::~SyncMutex()
{
	assert(state() == SYNC_STATE::UNLOCK);
}

SYNC_STATE SyncMutex::state()
{
	auto ret = WaitForSingleObject(handle(), WAIT_TIME_ZERO);
	if (ret == WAIT_OBJECT_0)
	{
		assert(Release());
		return SYNC_STATE::UNLOCK;
	}

	return SYNC_STATE::FULL_LOCK;
}

SyncSemaphore::SyncSemaphore(LONG max_count)
	: SyncHandle(::CreateSemaphore(nullptr, max_count, max_count, nullptr)), current_count_(max_count), max_count_(max_count)
{}

SyncSemaphore::SyncSemaphore(LONG init_count, LONG max_count)
	: SyncHandle(::CreateSemaphore(nullptr, init_count, max_count, nullptr)), current_count_(init_count), max_count_(max_count)
{
	assert(init_count <= max_count);
}

SyncSemaphore::~SyncSemaphore()
{
	assert(state() == SYNC_STATE::UNLOCK);
}

bool SyncSemaphore::Release(LONG& prev_count, LONG release_count)
{
	std::unique_lock<std::mutex> lock(mtx_);
	if (current_count_ + release_count > max_count_)
	{
		assert(false);
		return false;
	}

	if (ReleaseSemaphore(handle(), release_count, &prev_count) == false)
	{
		assert(false);
		return false;
	}

	prev_count += release_count;
	current_count_.store(prev_count);

	return true;
}

bool SyncSemaphore::Release(LONG release_count)
{
	LONG prev_count = 0;
	return Release(std::ref(prev_count), release_count);
}

SYNC_STATE SyncSemaphore::state()
{
	std::unique_lock<std::mutex> lock(mtx_);
	auto ret = WaitForSingleObject(handle(), WAIT_TIME_ZERO);
	if (ret == WAIT_OBJECT_0)
	{
		LONG key_count = 0;
		assert(ReleaseSemaphore(handle(), 1, &key_count));

		return ((key_count + 1) == max_count_) ? SYNC_STATE::UNLOCK : SYNC_STATE::SEGMENT_LOCK;
	}

	return SYNC_STATE::FULL_LOCK;
}

SyncEvent::SyncEvent(BOOL is_menual_reset, BOOL init_state)
	: SyncHandle(::CreateEvent(nullptr, is_menual_reset, init_state, nullptr)),
	is_menual_reset_(is_menual_reset), init_state_(init_state)
{}

SyncEvent::~SyncEvent()
{
	assert(state() == SYNC_STATE::UNLOCK);
}

SYNC_STATE SyncEvent::state()
{
	auto ret = WaitForSingleObject(handle(), WAIT_TIME_ZERO);
	if (ret == WAIT_OBJECT_0)
	{
		assert(Release());
		return SYNC_STATE::UNLOCK;
	}

	return SYNC_STATE::FULL_LOCK;
}