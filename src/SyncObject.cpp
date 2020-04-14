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
	if (ret >= WAIT_OBJECT_0 && ret <= WAIT_OBJECT_0 + max_count())
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
	if (state() != SYNC_STATE::UNLOCK)
		assert(Release());
}

DWORD SingleLock::Lock(DWORD timeout)
{
	ULONGLONG begin_tick = GetTickCount64();
	auto ret = sync_mutex_.Lock(timeout);
	if (ret != WAIT_OBJECT_0)
		return ret;

	ret = mutex_node_->Lock(GetTickCount64() - begin_tick);
	if (sync_mutex_.Release())
		return ret;

	assert(false);
	return WAIT_FAILED;
}

DWORD SingleLock::SpinLock(DWORD timeout)
{
	ULONGLONG begin_tick = GetTickCount64();
	auto ret = sync_mutex_.Lock(timeout);
	if (ret != WAIT_OBJECT_0)
		return ret;

	do
	{
		auto ret = Lock(WAIT_TIME_ZERO);
		if (ret == WAIT_OBJECT_0)
		{
			if (sync_mutex_.Release() == false)
			{
				assert(false);
				return WAIT_TIMEOUT;
			}
			
			return ret;
		}
	} while (timeout == INFINITE || begin_tick + timeout > GetTickCount64());

	assert(sync_mutex_.Release());
	return WAIT_TIMEOUT;
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
	: semaphore_node_(hub.make_node()), sync_semaphore_(hub->max_count())
{
	if (immedidate_lock)
		assert(Lock());
}

MultiLock::MultiLock(SyncSemaphoreNode& node, bool immedidate_lock)
	: semaphore_node_(node), sync_semaphore_(node->max_count())
{
	if (immedidate_lock)
		assert(Lock());
}

MultiLock::~MultiLock()
{
	if (state() != SYNC_STATE::UNLOCK)
		assert(Release());
}

DWORD MultiLock::Lock(DWORD timeout)
{
	return semaphore_node_->Lock(timeout);
}

DWORD MultiLock::SpinLock(DWORD timeout)
{
	const ULONGLONG begin_tick = GetTickCount64();
	
	do
	{
		auto ret = Lock(WAIT_TIME_ZERO);
		if (ret >= WAIT_OBJECT_0 && ret <= WAIT_OBJECT_0 + semaphore_node_->max_count())
			return ret;

	} while (timeout == INFINITE || begin_tick + timeout > GetTickCount64());

	return WAIT_TIMEOUT;
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
	: SingleLock(mutex_hub, false), MultiLock(semaphore_hub, false), lock_type_(LOCK_TYPE::MAX)
{}

RWLock::RWLock(SyncMutexNode& mutex_node, SyncSemaphoreHub& semaphore_hub)
	: SingleLock(mutex_node, false), MultiLock(semaphore_hub, false), lock_type_(LOCK_TYPE::MAX)
{}

RWLock::RWLock(SyncMutexHub& mutex_hub, SyncSemaphoreNode& semaphore_node)
	: SingleLock(mutex_hub, false), MultiLock(semaphore_node, false), lock_type_(LOCK_TYPE::MAX)
{}

RWLock::RWLock(SyncMutexNode& mutex_node, SyncSemaphoreNode& semaphore_node)
	: SingleLock(mutex_node, false), MultiLock(semaphore_node, false), lock_type_(LOCK_TYPE::MAX)
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

	auto ret = MultiLock::Lock(timeout);
	if (ret >= WAIT_OBJECT_0 && ret <= WAIT_OBJECT_0 + semaphore_node_->max_count())
		return true;

	return false;
}

bool RWLock::WriteLock(DWORD timeout)
{
	switch (state())
	{
	case SYNC_STATE::FULL_LOCK:
	case SYNC_STATE::SEGMENT_LOCK:
		return false;
	}

	return (SingleLock::Lock(timeout) == WAIT_OBJECT_0);
}

bool RWLock::ReadSpinLock(DWORD timeout)
{
	const ULONGLONG begin_tick = GetTickCount64();

	do
	{
		auto ret = WriteLock(WAIT_TIME_ZERO);
		if (ret != WAIT_OBJECT_0)
			continue;
			
		ret = ReadLock(WAIT_TIME_ZERO);
		if (ret < WAIT_OBJECT_0 && ret > WAIT_OBJECT_0 + semaphore_node_->max_count())
			continue;

		if (SingleLock::Release() == false)
		{
			if (MultiLock::Release())
			{
				assert(false);
				// ...
			}
				
			return WAIT_TIMEOUT;
		}

		return ret;

	} while (timeout == INFINITE || begin_tick + timeout > GetTickCount64());

	return WAIT_TIMEOUT;
}

bool RWLock::WriteSpinLock(DWORD timeout)
{
	const ULONGLONG begin_tick = GetTickCount64();

	do
	{
		auto ret = ReadLock(WAIT_TIME_ZERO);
		if (ret != WAIT_OBJECT_0 + semaphore_node_->max_count())
		{
			continue;
		}
			

		ret = WriteLock(WAIT_TIME_ZERO);
		if (ret != WAIT_OBJECT_0)
			continue;

		return ret;

	} while (timeout == INFINITE || begin_tick + timeout > GetTickCount64());

	return WAIT_TIMEOUT;
}