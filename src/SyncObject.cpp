#pragma once
#include "stdafx.h"
#include "SyncObject.h"

SyncObject::SyncObject(HANDLE handle)
	: handle_(handle)
{
}

SyncLock::SyncLock(HANDLE handle)
	: SyncObject(handle)
{
}

SyncLock::~SyncLock()
{
	assert(CloseHandle(handle_));
}

bool SyncLock::is_lock(ULONG cnt)
{
	const ULONG lockable_count = max_lock_count() - lock_count();

	if (lockable_count >= cnt)
		return true;

	assert(lockable_count < 0);

	return false;
}

bool SyncLock::try_lock(ULONG cnt)
{
	auto result = lock(cnt, 0);
	switch (result)
	{
	case WAIT_ABANDONED:
	case WAIT_TIMEOUT:
	case WAIT_FAILED:
		return false;
	case WAIT_OBJECT_0:
		return true;
	}

	return false;
}

SyncMutex::SyncMutex(BOOL b_init)
	: SyncLock(::CreateMutex(NULL, b_init, NULL))
{
}

SyncMutex::~SyncMutex()
{
	assert(lock_count() == 0);
}

DWORD SyncMutex::lock(ULONG cnt /* not used */, DWORD timeout)
{
	return WaitForSingleObject(handle_, timeout);
}

DWORD SyncMutex::unlock(ULONG cnt /* not used */)
{
	if (ReleaseMutex(handle_) == 0)
	{
		assert(false);
		return GetLastError();
	}

	return 0;
}

ULONG SyncMutex::lock_count()
{
	if (try_lock(1))
	{
		unlock(1);
		return 1;
	}

	return 0;
}

ULONG SyncMutex::max_lock_count() const
{
	return 1;
}

SyncSemaphore::SyncSemaphore(LONG max_lock_cnt)
	: SyncLock(::CreateSemaphore(nullptr, max_lock_cnt, max_lock_cnt, nullptr)), init_lock_cnt_(max_lock_cnt), max_lock_cnt_(max_lock_cnt)
{}

SyncSemaphore::SyncSemaphore(LONG init_lock_cnt, LONG max_lock_cnt)
	: SyncLock(::CreateSemaphore(nullptr, init_lock_cnt, max_lock_cnt, nullptr)), init_lock_cnt_(init_lock_cnt), max_lock_cnt_(max_lock_cnt)
{
	assert(init_lock_cnt <= max_lock_cnt);
}

SyncSemaphore::~SyncSemaphore()
{
	assert(init_lock_cnt_ < lock_count());
}

DWORD SyncSemaphore::lock(ULONG cnt, DWORD timeout)
{
	return WaitForMultipleObjects(cnt, &handle_, false, timeout);
}

DWORD SyncSemaphore::unlock(ULONG cnt)
{
	LONG lock_cnt = 0;
	if (ReleaseSemaphore(handle_, cnt, &lock_cnt) == 0)
	{
		assert(false);
		return GetLastError();
	}

	return 0;
}

ULONG SyncSemaphore::lock_count()
{
	LONG lock_cnt = 0;
	if (ReleaseSemaphore(handle_, 0, &lock_cnt) == false)
	{
		/// error
		return 0;
	}

	return static_cast<ULONG>(lock_cnt);
}

ULONG SyncSemaphore::max_lock_count() const
{
	return max_lock_cnt_;
}

SyncEvent::SyncEvent(BOOL is_menual_reset, BOOL init_state, const std::wstring name)
	: SyncObject(::CreateEvent(nullptr, is_menual_reset, init_state, name.empty() ? nullptr : name.c_str())),
	is_menual_reset_(is_menual_reset), init_state_(init_state)
{}

SyncEvent::~SyncEvent()
{
	///
}


DWORD SyncEvent::wait_signaled(DWORD timeout)
{
	return WaitForSingleObject(handle_, timeout);
}

DWORD SyncEvent::raise_signaled()
{
	if (SetEvent(handle_) == 0)
	{
		assert(false);
		return GetLastError();
	}

	return 0;
}