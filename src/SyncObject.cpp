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
	: signaled_count_(0)
{
}

LockBase::~LockBase()
{
	assert(signaled_count_ == 0);
}

bool LockBase::Lock(DWORD timeout)
{
	auto ret = _Lock(timeout);
	if (ret)
		signaled_count_ += 1;

	return ret;
}

bool LockBase::Release()
{
	if (signaled_count_ == 0)
	{
		assert(false);
		return false;
	}

	return _Release();;
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
	assert(_Destory());
}

bool SingleLock::_Lock(DWORD timeout)
{
	assert(signaled_count_ == 0);
	DWORD ret = mutex_node_->Lock(timeout);
	switch (ret)
	{	
	case WAIT_ABANDONED:
		assert(false);
	case WAIT_OBJECT_0:
		signaled_count_ = 1;
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
			return true;
		
	} while (timeout == INFINITE || static_cast<DWORD>(GetTickCount() - begin_tick) / SECONDS_TO_TICK > timeout);

	return false;
}

bool SingleLock::_Release()
{
	auto ret = mutex_node_->Release();
	if (ret == false)
		assert(ret);
	else
		signaled_count_ = 0;
		
	return ret;
}

bool SingleLock::_Destory()
{
	if (signaled_count_ != 0)
		return _Release();

	return true;
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
	assert(_Destory());
}

bool MultiLock::_Lock(DWORD timeout)
{
	DWORD ret = semaphore_node_->Lock(timeout);
	if (ret == WAIT_OBJECT_0)
		return true;
	
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
	auto ret = semaphore_node_->Release();
	if (ret == false)
		assert(false);
	else
		signaled_count_ -= 1;
	
	return ret;
}

bool MultiLock::_Destory()
{
	while (signaled_count_ > 0) 
	{ 
		auto ret = _Release(); 
		if (ret == false)
			return false;
	}

	return true;
}

bool MultiLock::_Release(LONG& prev_count, LONG release_count)
{
	auto ret = semaphore_node_->_Release(std::ref(prev_count), release_count);
	if (ret == false)
		assert(false);
	else
		signaled_count_ -= release_count;

	return ret;
}

RWLock::RWLock(SyncSemaphoreHub& semaphore_hub)
	: MultiLock(semaphore_hub)
{}

RWLock::RWLock(SyncSemaphoreNode& semaphore_node)
	: MultiLock(semaphore_node)
{}

RWLock::~RWLock()
{
	assert(_Destory());
}

bool RWLock::WriteLock(DWORD timeout)
{
	ULONGLONG begin_tick = GetTickCount64();
	DWORD remain_time = INFINITE;
	{
		ULONGLONG process_time = (GetTickCount64() - begin_tick) / SECONDS_TO_TICK;
		remain_time = (timeout == INFINITE) ? INFINITE : (timeout > process_time) ? timeout - static_cast<DWORD>(process_time) : WAIT_TIME_ZERO;
	}

	LONG max_count = MultiLock::max_count();	
	for (LONG i = 0; i < max_count; ++i)
	{
		begin_tick = GetTickCount64();
		if (Lock(remain_time) != WAIT_OBJECT_0)
			break;
		
		ULONGLONG process_time = (GetTickCount64() - begin_tick) / SECONDS_TO_TICK;
		remain_time = (timeout == INFINITE) ? INFINITE : (timeout > process_time) ? timeout - static_cast<DWORD>(process_time) : WAIT_TIME_ZERO;
	}

	if (signaled_count_ != max_count)
	{
		LONG prev_count = 0;
		assert(_Release(std::ref(prev_count), signaled_count_));
		return false;
	}

	return true;
}

bool RWLock::ReadLock(DWORD timeout)
{
	ULONGLONG begin_tick = GetTickCount64();
	DWORD remain_time = INFINITE;
	{
		ULONGLONG process_time = (GetTickCount64() - begin_tick) / SECONDS_TO_TICK;
		remain_time = (timeout == INFINITE) ? INFINITE : (timeout > process_time) ? timeout - static_cast<DWORD>(process_time) : WAIT_TIME_ZERO;
	}

	auto ret = Lock(remain_time);
	if (ret != WAIT_OBJECT_0)
		return false;
	
	return true;
}

bool RWLock::WriteRelease()
{
	if (signaled_count_ != MultiLock::max_count())
		return false;

	if (_Release(signaled_count_) == false)
		return false;

	return true;
}

bool RWLock::ReadRelease()
{
	if (signaled_count_ == 0)
		return false;

	if (_Release() == false)
		return false;

	return true;
}