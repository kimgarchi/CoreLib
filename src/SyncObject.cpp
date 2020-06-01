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
}

DWORD SyncEvent::Lock(DWORD timeout)
{
	/*
	assert(is_menual_reset_ == false ? true : ResetEvent(handle()));

	event menual... dif?

	*/
	
	return SyncHandle::Lock(timeout);
}

LockBase::LockBase()
	: signaled_(false)
{
}

SingleLock::SingleLock(SyncMutexHub& hub, bool immediate_lock)
	: mutex_node_(hub.make_node())
{
	if (immediate_lock == false)
		return;

	assert(Lock());
}

SingleLock::SingleLock(SyncMutexNode& node, bool immediate_lock)
	: mutex_node_(node)
{
	if (immediate_lock == false)
		return;

	assert(Lock());
}

SingleLock::~SingleLock()
{
	if (signaled().load())
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
		signaled().store(true);
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
			signaled().store(true);
			return true;
		}

	} while (timeout == INFINITE || static_cast<DWORD>(GetTickCount() - begin_tick) / SECONDS_TO_TICK > timeout);

	return false;
}

bool SingleLock::_Release()
{
	if (signaled().load())
	{
		auto ret = mutex_node_->Release();
		if (ret == false)
			assert(ret);
		else
			signaled().store(false);
		
		return ret;
	}

	return false;
}

MultiLock::MultiLock(SyncSemaphoreHub& hub, bool immediate_lock)
	: semaphore_node_(hub.make_node())
{
	if (immediate_lock == false)
		return;

	assert(Lock());
}

MultiLock::MultiLock(SyncSemaphoreNode& node, bool immediate_lock)
	: semaphore_node_(node)
{
	if (immediate_lock == false)
		return;

	assert(Lock());
}

MultiLock::~MultiLock()
{
	if (signaled().load())
		assert(_Release());
}

bool MultiLock::_Lock(DWORD timeout)
{
	DWORD ret = semaphore_node_->Lock(timeout);
	if (ret == WAIT_OBJECT_0)
	{
		signaled().store(true);
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
	if (signaled().load())
	{
		auto ret = semaphore_node_->Release();
		if (ret == false)
			assert(false);
		else
			signaled().store(false);
	
		return ret;
	}

	return true;
}

bool MultiLock::_Release(LONG& prev_count, LONG release_count)
{
	auto ret = semaphore_node_->_Release(std::ref(prev_count), release_count);
	if (ret)
		signaled().store(false);

	return ret;
}

RWLock::RWLock(SyncMutexHub& mutex_hub, SyncSemaphoreHub& semaphore_hub)
	: mutex_node_(mutex_hub.make_node()), semaphore_node_(semaphore_hub.make_node()), signaled_count_(0)
{
}

RWLock::RWLock(SyncMutexNode& mutex_node, SyncSemaphoreHub& semaphore_hub)
	: mutex_node_(mutex_node), semaphore_node_(semaphore_hub.make_node()), signaled_count_(0)
{}

RWLock::RWLock(SyncMutexHub& mutex_hub, SyncSemaphoreNode& semaphore_node)
	: mutex_node_(mutex_hub.make_node()), semaphore_node_(semaphore_node), signaled_count_(0)
{}

RWLock::RWLock(SyncMutexNode& mutex_node, SyncSemaphoreNode& semaphore_node)
	: mutex_node_(mutex_node), semaphore_node_(semaphore_node), signaled_count_(0)
{}

RWLock::~RWLock()
{
	if (signaled_count_.load() > 0)
		assert(semaphore_node_->_Release(signaled_count_.load()));
}

bool RWLock::WriteLock(DWORD timeout)
{
	ULONGLONG begin_tick = GetTickCount64();	
	SingleLock single_lock(mutex_node_, false);
	if (single_lock.Lock(timeout) == false)
		return false;

	DWORD remain_time = INFINITE;

	{
		ULONGLONG process_time = (GetTickCount64() - begin_tick) / SECONDS_TO_TICK;
		remain_time = (timeout == INFINITE) ? INFINITE : (timeout > process_time) ? timeout - static_cast<DWORD>(process_time) : WAIT_TIME_ZERO;
	}

	const auto max_count = semaphore_node_->max_count();	
	for (LONG i = 0; i < max_count; ++i)
	{
		begin_tick = GetTickCount64();
		if (semaphore_node_->Lock(remain_time) != WAIT_OBJECT_0)
			break;

		{
			ULONGLONG process_time = (GetTickCount64() - begin_tick) / SECONDS_TO_TICK;
			remain_time = (timeout == INFINITE) ? INFINITE : (timeout > process_time) ? timeout - static_cast<DWORD>(process_time) : WAIT_TIME_ZERO;
		}

		signaled_count_.fetch_add(1);
	}

	if (signaled_count_.load() != max_count)
	{
		assert(semaphore_node_->_Release(signaled_count_.load()));
		signaled_count_.fetch_sub(signaled_count_.load());
		return false;
	}

	return true;
}

bool RWLock::ReadLock(DWORD timeout)
{
	ULONGLONG begin_tick = GetTickCount64();
	SingleLock single_lock(mutex_node_, false);
	if (single_lock.Lock(timeout) == false)
		return false;

	DWORD remain_time = INFINITE;

	{
		ULONGLONG process_time = (GetTickCount64() - begin_tick) / SECONDS_TO_TICK;
		remain_time = (timeout == INFINITE) ? INFINITE : (timeout > process_time) ? timeout - static_cast<DWORD>(process_time) : WAIT_TIME_ZERO;
	}

	auto ret = semaphore_node_->Lock(remain_time);
	if (ret != WAIT_OBJECT_0)
		return false;
	
	signaled_count_.fetch_add(1);
	return true;
}

bool RWLock::WriteRelease()
{
	if (signaled_count_.load() != semaphore_node_->max_count())
		return false;

	if (semaphore_node_->_Release(signaled_count_.load()) == false)
		return false;

	signaled_count_.fetch_sub(signaled_count_.load());

	return true;
}

bool RWLock::ReadRelease()
{
	if (signaled_count_.load() == 0)
		return false;

	if (semaphore_node_->Release() == false)
		return false;

	signaled_count_.fetch_sub(1);

	return true;
}