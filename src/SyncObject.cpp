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

bool SyncSemaphore::_Release(LONG& prev_count, LONG release_count)
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

bool SyncSemaphore::_Release(LONG release_count)
{
	LONG prev_count = 0;
	return _Release(std::ref(prev_count), release_count);
}

SYNC_STATE SyncSemaphore::state()
{
	auto ret = Lock(WAIT_TIME_ZERO);
	if (ret >= WAIT_OBJECT_0 && ret <= WAIT_OBJECT_0 + max_count())
	{
		LONG key_count = 0;
		assert(ReleaseSemaphore(handle(), 1, &key_count));

		return (key_count < max_count_) ? SYNC_STATE::UNLOCK : SYNC_STATE::SEGMENT_LOCK;
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

SingleLock::SingleLock(SyncMutexHub& hub)
	: mutex_node_(hub.make_node())
{}

SingleLock::SingleLock(SyncMutexNode& node)
	: mutex_node_(node)
{}

SingleLock::~SingleLock()
{
	if (state() != SYNC_STATE::UNLOCK)
		assert(Release());
}

DWORD SingleLock::_SpinLock(DWORD timeout)
{
	ULONGLONG begin_tick = GetTickCount64();
	do
	{
		auto ret = Lock(WAIT_TIME_ZERO);
		if (ret == WAIT_OBJECT_0)
			return ret;

	} while (timeout == INFINITE || begin_tick + timeout > GetTickCount64());

	return WAIT_TIMEOUT;
}

bool SingleLock::_Release()
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

MultiLock::MultiLock(SyncSemaphoreHub& hub)
	: semaphore_node_(hub.make_node())
{}

MultiLock::MultiLock(SyncSemaphoreNode& node)
	: semaphore_node_(node)
{}

MultiLock::~MultiLock()
{
	if (state() != SYNC_STATE::UNLOCK)
	{
		while (state() != SYNC_STATE::UNLOCK)
			assert(Release());
	}	
}

DWORD MultiLock::_SpinLock(DWORD timeout)
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

bool MultiLock::_Release()
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
	: SingleLock(mutex_hub), MultiLock(semaphore_hub)
{}

RWLock::RWLock(SyncMutexNode& mutex_node, SyncSemaphoreHub& semaphore_hub)
	: SingleLock(mutex_node), MultiLock(semaphore_hub)
{}

RWLock::RWLock(SyncMutexHub& mutex_hub, SyncSemaphoreNode& semaphore_node)
	: SingleLock(mutex_hub), MultiLock(semaphore_node)
{}

RWLock::RWLock(SyncMutexNode& mutex_node, SyncSemaphoreNode& semaphore_node)
	: SingleLock(mutex_node), MultiLock(semaphore_node)
{}

SYNC_STATE RWLock::state()
{
	{
		auto ret = WaitForSingleObject(mutex_node_->handle(), WAIT_TIME_ZERO);
		if (ret != WAIT_OBJECT_0)
		{
			mutex_node_->Release();
			return SYNC_STATE::FULL_LOCK;
		}
	}
	
	SYNC_STATE state = SYNC_STATE::FULL_LOCK;

	{
		auto ret = WaitForSingleObject(semaphore_node_->handle(), WAIT_TIME_ZERO);
		if (ret == WAIT_OBJECT_0)
			state = SYNC_STATE::UNLOCK;
		else if (ret > WAIT_OBJECT_0 && ret <= WAIT_OBJECT_0 + semaphore_node_->max_count())
			state = SYNC_STATE::SEGMENT_LOCK;
	}

	assert(mutex_node_->Release());
	assert(semaphore_node_->Release());

	return state;
}
/*
DWORD RWLock::ReadLock(DWORD timeout)
{
	auto ret = WriteLock(timeout);
	
	auto ret = MultiLock::_Lock(timeout);
	if (ret >= WAIT_OBJECT_0 && ret <= WAIT_OBJECT_0 + semaphore_node_->max_count())
		return true;

	return false;
}

DWORD RWLock::WriteLock(DWORD timeout)
{
	auto ret = SingleLock::Lock(timeout);
	if (ret != WAIT_OBJECT_0)
		return false;
	
	return (ret == WAIT_OBJECT_0);
}

DWORD RWLock::ReadSpinLock(DWORD timeout)
{
	const ULONGLONG begin_tick = GetTickCount64();

	do
	{
		{
			auto ret = WriteLock(WAIT_TIME_ZERO);
			if (ret != WAIT_OBJECT_0)
				continue;
		}
		
		{
			auto ret = MultiLock::Lock(timeout);
			if (ret < WAIT_OBJECT_0 && ret >(WAIT_OBJECT_0 + semaphore_node_->max_count()))
				continue;
		}

		if (SingleLock::Release() == false)
		{
			if (MultiLock::Release())
				assert(false);
				
			return false;
		}

		return true;

	} while (timeout == INFINITE || begin_tick + timeout > GetTickCount64());

	return false;
}

DWORD RWLock::WriteSpinLock(DWORD timeout)
{
	const ULONGLONG begin_tick = GetTickCount64();

	do
	{
		{
			auto ret = MultiLock::Lock(WAIT_TIME_ZERO);
			if (ret != WAIT_OBJECT_0 + semaphore_node_->max_count())
				continue;
		}
		
		{
			auto ret = SingleLock::Lock(WAIT_TIME_ZERO);
			if (ret != WAIT_OBJECT_0)
				continue;
		}
		
		return true;

	} while (timeout == INFINITE || begin_tick + timeout > GetTickCount64());

	return false;
}
*/