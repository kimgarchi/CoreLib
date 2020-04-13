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

DWORD SyncHandle::Lock(DWORD timeout)
{
	return WaitForSingleObject(handle(), timeout);
}

SyncMutex::SyncMutex()
	: SyncHandle(::CreateMutex(nullptr, false, nullptr))
{}

SyncMutex::SyncMutex(const SyncMutex& mutex)
	: SyncHandle(const_cast<SyncMutex&>(mutex).handle())
{}

SyncMutex::~SyncMutex()
{
	assert(state() == SYNC_STATE::UNLOCK);
}

SYNC_STATE SyncMutex::state()
{
	auto ret = Lock(WAIT_TIME_ZERO);
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
	auto ret = Lock(WAIT_TIME_ZERO);
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
	auto ret = Lock(WAIT_TIME_ZERO);
	if (ret == WAIT_OBJECT_0)
	{
		assert(Release());
		return SYNC_STATE::UNLOCK;
	}

	return SYNC_STATE::FULL_LOCK;
}

SingleLock::SingleLock(SyncMutexHub& hub, bool immedidate_lock)
	: mutex_node_(hub.make_node())
{
	if (immedidate_lock)
		assert(Lock());
}

SingleLock::SingleLock(SyncMutexNode& node, bool immedidate_lock)
	: mutex_node_(node)
{
	if (immedidate_lock)
		assert(Lock());
}

SingleLock::~SingleLock()
{
	assert(Release());	
}

bool SingleLock::Release()
{
	switch (mutex_node_->state())
	{
	case SYNC_STATE::FULL_LOCK:
		assert(mutex_node_->Release());
		break;
	case SYNC_STATE::UNLOCK:
		break;
	default:
		assert(false);
		return false;
	}

	return true;
}

MultiLock::MultiLock(SyncSemaphoreHub& hub, bool immedidate_lock)
	: semaphore_node_(hub.make_node())
{
	if (immedidate_lock)
		assert(Lock());
}

MultiLock::MultiLock(SyncSemaphoreNode& node, bool immedidate_lock)
	: semaphore_node_(node)
{
	if (immedidate_lock)
		assert(Lock());
}

MultiLock::~MultiLock()
{
	assert(Release());
}

bool MultiLock::Release()
{
	switch (semaphore_node_->state())
	{
	case SYNC_STATE::SEGMENT_LOCK:
	case SYNC_STATE::FULL_LOCK:
		assert(semaphore_node_->Release());
		break;
	case SYNC_STATE::UNLOCK:
		break;
	default:
		assert(false);
		return false;
	}

	return true;
}

RWLock::RWLock(SyncMutexHub& mutex_hub, SyncSemaphoreHub& semaphore_hub)
	: SingleLock(mutex_hub, false), MultiLock(semaphore_hub, false)
{}

RWLock::RWLock(SyncMutexNode& mutex_node, SyncSemaphoreHub& semaphore_hub)
	: SingleLock(mutex_node, false), MultiLock(semaphore_hub, false)
{}

RWLock::RWLock(SyncMutexHub& mutex_hub, SyncSemaphoreNode& semaphore_node)
	: SingleLock(mutex_hub, false), MultiLock(semaphore_node, false)
{}

RWLock::RWLock(SyncMutexNode& mutex_node, SyncSemaphoreNode& semaphore_node)
	: SingleLock(mutex_node, false), MultiLock(semaphore_node, false)
{}

SYNC_STATE RWLock::state()
{
	auto state = SingleLock::state();
	if (state == SYNC_STATE::FULL_LOCK)
		return state;

	return MultiLock::state();
}

bool RWLock::ReadLock(DWORD timeout)
{
	switch (state())
	{
	case SYNC_STATE::FULL_LOCK:
		return false;
	}

	return (MultiLock::Lock() == WAIT_OBJECT_0);
}

bool RWLock::WriteLock(DWORD timeout)
{
	switch (state())
	{
	case SYNC_STATE::FULL_LOCK:
	case SYNC_STATE::SEGMENT_LOCK:
		return false;
	}

	return (SingleLock::Lock() == WAIT_OBJECT_0);
}
