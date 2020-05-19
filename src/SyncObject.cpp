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
	: SyncHandle(::CreateSemaphore(nullptr, max_count, max_count, nullptr)), current_count_(max_count), init_count_(max_count), max_count_(max_count)
{}

SyncSemaphore::SyncSemaphore(LONG init_count, LONG max_count)
	: SyncHandle(::CreateSemaphore(nullptr, init_count, max_count, nullptr)), current_count_(init_count), init_count_(init_count), max_count_(max_count)
{
	assert(init_count <= max_count);
}

SyncSemaphore::SyncSemaphore(const SyncSemaphore& semaphore)
	: SyncHandle(const_cast<SyncSemaphore&>(semaphore).handle()), current_count_(semaphore.init_count_), init_count_(semaphore.init_count_), max_count_(semaphore.max_count_)
{
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

SyncEvent::SyncEvent(const SyncEvent& event)
	: SyncHandle(const_cast<SyncEvent&>(event).handle()), 
	is_menual_reset_(event.is_menual_reset_), init_state_(event.init_state_)
{
}

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

SingleLock::SingleLock(SyncMutexHub& hub, bool immediate_lock)
	: mutex_node_(hub.make_node())
{
	if (immediate_lock == false)
		return;

	assert(Lock() == SYNC_STATE::UNLOCK);
}

SingleLock::SingleLock(SyncMutexNode& node, bool immediate_lock)
	: mutex_node_(node)
{
	if (immediate_lock == false)
		return;

	assert(Lock() == SYNC_STATE::UNLOCK);
}

SingleLock::~SingleLock()
{
	if (state() != SYNC_STATE::UNLOCK)
		assert(Release());
}

SYNC_STATE SingleLock::_Lock(DWORD timeout)
{
	auto ret = mutex_node_->Lock(timeout);
	switch (ret)
	{
	case WAIT_OBJECT_0:
		return SYNC_STATE::UNLOCK;
	}

	return SYNC_STATE::FULL_LOCK;
}

SYNC_STATE SingleLock::_SpinLock(DWORD timeout)
{
	ULONGLONG begin_tick = GetTickCount64();
	do
	{
		auto ret = mutex_node_->Lock(WAIT_TIME_ZERO);
		if (ret == WAIT_OBJECT_0)
			return SYNC_STATE::UNLOCK;

	} while (timeout == INFINITE || static_cast<DWORD>(GetTickCount() - begin_tick) / SECONDS_TO_TICK > timeout);

	return SYNC_STATE::FULL_LOCK;
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

MultiLock::MultiLock(SyncSemaphoreHub& hub, bool immediate_lock)
	: semaphore_node_(hub.make_node())
{
	if (immediate_lock == false)
		return;

	assert(Lock() != SYNC_STATE::FULL_LOCK);
}

MultiLock::MultiLock(SyncSemaphoreNode& node, bool immediate_lock)
	: semaphore_node_(node)
{
	if (immediate_lock == false)
		return;

	assert(Lock() != SYNC_STATE::FULL_LOCK);
}

MultiLock::~MultiLock()
{
	if (state() != SYNC_STATE::UNLOCK)
	{
		while (state() != SYNC_STATE::UNLOCK)
			assert(Release());
	}	
}

SYNC_STATE MultiLock::_Lock(DWORD timeout)
{
	auto ret = semaphore_node_->Lock(timeout);
	if (ret == WAIT_OBJECT_0)
		return SYNC_STATE::UNLOCK;
	else if (ret > WAIT_OBJECT_0 && ret <= WAIT_OBJECT_0 + semaphore_node_->max_count())
		return SYNC_STATE::SEGMENT_LOCK;

	return SYNC_STATE::FULL_LOCK;
}

SYNC_STATE MultiLock::_SpinLock(DWORD timeout)
{
	const ULONGLONG begin_tick = GetTickCount64();
	
	do
	{
		auto ret = semaphore_node_->Lock(WAIT_TIME_ZERO);
		if (ret == WAIT_OBJECT_0)
			return SYNC_STATE::UNLOCK;
		else if (ret > WAIT_OBJECT_0 && ret <= WAIT_OBJECT_0 + semaphore_node_->max_count())
			return SYNC_STATE::SEGMENT_LOCK;

	} while (timeout == INFINITE || static_cast<DWORD>(GetTickCount() - begin_tick) / SECONDS_TO_TICK > timeout);

	return SYNC_STATE::FULL_LOCK;
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

SYNC_STATE RWLock::ReadLock(DWORD timeout)
{
	const ULONGLONG begin_tick = GetTickCount64();

	switch (SingleLock::Lock(timeout))
	{
	case SYNC_STATE::UNLOCK:
		break;
	default:
		assert(SingleLock::Release());
		return SYNC_STATE::FULL_LOCK;
	}

	auto ret = MultiLock::Lock((timeout == INFINITE) ? INFINITE : static_cast<DWORD>((GetTickCount() - begin_tick) / SECONDS_TO_TICK));
	assert(SingleLock::Release());

	return ret;
}

SYNC_STATE RWLock::WriteLock(DWORD timeout)
{
	const ULONGLONG begin_tick = GetTickCount64();

	auto ret = MultiLock::Lock(timeout);
	switch (ret)
	{
	case SYNC_STATE::SEGMENT_LOCK:
	case SYNC_STATE::FULL_LOCK:
		assert(MultiLock::Release());
		return ret;
	}

	ret = SingleLock::Lock((timeout == INFINITE) ? INFINITE : static_cast<DWORD>((GetTickCount() - begin_tick) / SECONDS_TO_TICK));
	assert(MultiLock::Release());
	
	return ret;
}

SYNC_STATE RWLock::ReadSpinLock(DWORD timeout)
{
	ULONGLONG begin_tick = GetTickCount64();

	do
	{
		switch (SingleLock::Lock(WAIT_TIME_ZERO))
		{
		case SYNC_STATE::FULL_LOCK:
			assert(SingleLock::Release());
			continue;
		}

		auto ret = MultiLock::Lock(WAIT_TIME_ZERO);
		if (ret == SYNC_STATE::FULL_LOCK)
			continue;
		
		assert(SingleLock::Release());		
		return ret;

	} while (timeout == INFINITE || static_cast<DWORD>(GetTickCount() - begin_tick) / SECONDS_TO_TICK > timeout);

	return SYNC_STATE::FULL_LOCK;
}

SYNC_STATE RWLock::WriteSpinLock(DWORD timeout)
{
	ULONGLONG begin_tick = GetTickCount64();

	do
	{
		auto ret = MultiLock::Lock(WAIT_TIME_ZERO);
		if (ret == SYNC_STATE::FULL_LOCK)
		{
			assert(MultiLock::Release());
			continue;
		}

		switch (SingleLock::Lock(WAIT_TIME_ZERO))
		{
		case SYNC_STATE::FULL_LOCK:
			continue;
		}

		assert(MultiLock::Release());
		return ret;

	} while (timeout == INFINITE || static_cast<DWORD>(GetTickCount() - begin_tick) / SECONDS_TO_TICK > timeout);

	return SYNC_STATE::FULL_LOCK;
}