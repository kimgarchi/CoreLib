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

SyncMutex::SyncMutex(BOOL b_init)
	: SyncHandle(::CreateMutex(NULL, b_init, NULL))
{}

SyncMutex::SyncMutex(const SyncMutex& mutex)
	: SyncHandle(const_cast<SyncMutex&>(mutex).handle())
{}

SyncSemaphore::SyncSemaphore(LONG max_count)
	: SyncHandle(::CreateSemaphore(nullptr, max_count, max_count, nullptr)), init_count_(max_count), max_count_(max_count)
{}

SyncSemaphore::SyncSemaphore(LONG init_count, LONG max_count)
	: SyncHandle(::CreateSemaphore(nullptr, init_count, max_count, nullptr)), init_count_(init_count), max_count_(max_count)
{
	assert(init_count <= max_count);
}

SyncSemaphore::SyncSemaphore(const SyncSemaphore& semaphore)
	: SyncHandle(const_cast<SyncSemaphore&>(semaphore).handle()), init_count_(semaphore.init_count_), max_count_(semaphore.max_count_)
{
}

bool SyncSemaphore::_Release(LONG& prev_count, LONG release_count)
{
	if (ReleaseSemaphore(handle(), release_count, &prev_count) == false)
	{
		assert(false);
		return false;
	}

	prev_count += release_count;

	return true;
}

bool SyncSemaphore::_Release(LONG release_count)
{
	LONG prev_count = 0;
	return _Release(std::ref(prev_count), release_count);
}
/*
SYNC_STATE SyncSemaphore::state()
{
	auto ret = Lock(WAIT_TIME_ZERO);
	if (ret >= WAIT_OBJECT_0 && ret <= WAIT_OBJECT_0 + max_count())
	{
		LONG key_count = 0;
		assert(_Release(std::ref(key_count), 1));

		return (key_count == max_count_) ? SYNC_STATE::UNLOCK : SYNC_STATE::SEGMENT_LOCK;
	}

	return SYNC_STATE::FULL_LOCK;
}*/

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
	//assert(state() == SYNC_STATE::UNLOCK);
}

SingleLock::SingleLock(SyncMutexHub& hub, bool immediate_lock)
	: mutex_node_(hub.make_node())
{
	if (immediate_lock == false)
		return;

	assert(Lock());
}

SingleLock::SingleLock(SyncMutexNode& node, bool immediate_lock)
	: mutex_node_(node), signaled_(false)
{
	if (immediate_lock == false)
		return;

	assert(Lock());
}

SingleLock::~SingleLock()
{
	if (signaled_.load())
		assert(_Release());
}

bool SingleLock::_Lock(DWORD timeout)
{
	DWORD ret = mutex_node_->Lock(timeout);
	switch (ret)
	{	
	case WAIT_ABANDONED:
		assert(false);
	case WAIT_OBJECT_0:
		signaled_.store(true);
		return true;
	}

	return false;
}

bool SingleLock::_SpinLock(DWORD timeout)
{
	ULONGLONG begin_tick = GetTickCount64();
	do
	{
		if (_Lock(WAIT_TIME_ZERO))
		{
			signaled_.store(true);
			return true;
		}

	} while (timeout == INFINITE || static_cast<DWORD>(GetTickCount() - begin_tick) / SECONDS_TO_TICK > timeout);

	return false;
}

bool SingleLock::_Release()
{
	if (signaled_.load())
	{
		auto ret = mutex_node_->Release();
		if (ret == false)
			assert(ret);
		else
			signaled_.store(false);
		
		return ret;
	}

	return false;
}

MultiLock::MultiLock(SyncSemaphoreHub& hub, bool immediate_lock)
	: semaphore_node_(hub.make_node()), signaled_(false)
{
	if (immediate_lock == false)
		return;

	assert(Lock());
}

MultiLock::MultiLock(SyncSemaphoreNode& node, bool immediate_lock)
	: semaphore_node_(node), signaled_(false)
{
	if (immediate_lock == false)
		return;

	assert(Lock());
}

MultiLock::~MultiLock()
{
	if (signaled_.load())
		assert(_Release());
}

bool MultiLock::_Lock(DWORD timeout)
{
	DWORD ret = semaphore_node_->Lock(timeout);
	if (ret == WAIT_OBJECT_0)
	{
		signaled_.store(true);
		return true;
	}

	return false;
}

bool MultiLock::_SpinLock(DWORD timeout)
{
	const ULONGLONG begin_tick = GetTickCount64();
	
	do
	{
		if (_Lock(WAIT_TIME_ZERO))
			return true;

	} while (timeout == INFINITE || static_cast<DWORD>(GetTickCount() - begin_tick) / SECONDS_TO_TICK > timeout);

	return false;
}

bool MultiLock::_Release()
{
	if (signaled_.load())
	{
		auto ret = semaphore_node_->Release();
		if (ret == false)
			assert(false);
		else
			signaled_.store(false);
	
		return ret;
	}

	return true;
}

RWLock::RWLock(SyncMutexHub& mutex_hub, SyncSemaphoreHub& semaphore_hub)
	: single_lock_(mutex_hub, false), multi_lock_(semaphore_hub, false)
{}

RWLock::RWLock(SyncMutexNode& mutex_node, SyncSemaphoreHub& semaphore_hub)
	: single_lock_(mutex_node, false), multi_lock_(semaphore_hub, false)
{}

RWLock::RWLock(SyncMutexHub& mutex_hub, SyncSemaphoreNode& semaphore_node)
	: single_lock_(mutex_hub, false), multi_lock_(semaphore_node, false)
{}

RWLock::RWLock(SyncMutexNode& mutex_node, SyncSemaphoreNode& semaphore_node)
	: single_lock_(mutex_node, false), multi_lock_(semaphore_node, false)
{}

bool RWLock::ReadLock(DWORD timeout)
{
	const ULONGLONG begin_tick = GetTickCount64();

	if (single_lock_.Lock(timeout))
	{
		assert(single_lock_.Release());
		return false;
	}

	auto ret = multi_lock_.Lock((timeout == INFINITE) ? INFINITE : static_cast<DWORD>((GetTickCount() - begin_tick) / SECONDS_TO_TICK));
	assert(single_lock_.Release());

	return true;
}

bool RWLock::WriteLock(DWORD timeout)
{
	const ULONGLONG begin_tick = GetTickCount64();

	if (multi_lock_.Lock(timeout) == false)
	{
		assert(multi_lock_.Release());
		return false;
	}

	auto ret = single_lock_.Lock((timeout == INFINITE) ? INFINITE : static_cast<DWORD>((GetTickCount() - begin_tick) / SECONDS_TO_TICK));
	assert(multi_lock_.Release());
	
	return ret;
}

bool RWLock::ReadSpinLock(DWORD timeout)
{
	ULONGLONG begin_tick = GetTickCount64();

	do
	{
		if (single_lock_.Lock(WAIT_TIME_ZERO) == false)
		{
			assert(single_lock_.Release());
			continue;
		}

		if (multi_lock_.Lock(WAIT_TIME_ZERO) == false)
			continue;
		
		assert(single_lock_.Release());
		return true;

	} while (timeout == INFINITE || static_cast<DWORD>(GetTickCount() - begin_tick) / SECONDS_TO_TICK > timeout);

	return false;
}

bool RWLock::WriteSpinLock(DWORD timeout)
{
	ULONGLONG begin_tick = GetTickCount64();

	do
	{
		if (multi_lock_.Lock(WAIT_TIME_ZERO) == false)
		{
			assert(multi_lock_.Release());
			continue;
		}

		if (single_lock_.Lock(WAIT_TIME_ZERO) == false)
			continue;
		
		assert(multi_lock_.Release());
		return true;

	} while (timeout == INFINITE || static_cast<DWORD>(GetTickCount() - begin_tick) / SECONDS_TO_TICK > timeout);

	return false;
}