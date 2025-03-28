#include "stdafx.h"
#include "LockObject.h"

LockBase::LockBase()
	: locked_cnt_(0)
{
}

LockBase::~LockBase()
{
	assert(locked_cnt_ == 0);
}

SingleLock::SingleLock(SyncMutex& sync_mutex, bool immediate_lock, DWORD timeout)
	: sync_mutex_(sync_mutex)
{
	if (immediate_lock == false)
		return;

	assert(lock(timeout) == 0);
}

SingleLock::~SingleLock()
{
	if (locked_cnt_ != 0)
		assert(unlock() == 0);
}

DWORD SingleLock::lock(DWORD timeout)
{
	const DWORD ret = sync_mutex_.lock(1, timeout);
	if (ret == WAIT_OBJECT_0)
	{
		locked_cnt_ = 1;
		return 0;
	}
	
	return ret;
}

DWORD SingleLock::spin_lock(DWORD timeout)
{
	const ULONGLONG start_tick = GetTickCount64();
	ULONGLONG cur_tick = start_tick;
	do 
	{
		const DWORD ret = lock(0);
		if (ret == 0)
			break;

		cur_tick = GetTickCount64();

	} while (timeout != INFINITE && cur_tick - start_tick > timeout);

	return WAIT_TIMEOUT;
}

DWORD SingleLock::unlock()
{
	const DWORD ret = sync_mutex_.unlock(1);
	if (ret == 0)
	{
		locked_cnt_ = 0;
		return 0;
	}
	
	return ret;
}

MultiLock::MultiLock(SyncSemaphore& sync_semaphore, bool immediate_lock, ULONG lock_cnt, DWORD timeout)
	: sync_semaphore_(sync_semaphore)
{
	if (immediate_lock == false)
		return;

	assert(lock(lock_cnt, timeout) == 0);
}

MultiLock::~MultiLock()
{
	if (locked_cnt_ != 0)
		assert(unlock(locked_cnt_) == 0);
}

DWORD MultiLock::lock(ULONG lock_cnt, DWORD timeout)
{
	const DWORD ret = sync_semaphore_.lock(lock_cnt, timeout);
	if (ret == WAIT_OBJECT_0)
	{
		locked_cnt_ = lock_cnt;
		return 0;
	}

	return ret;
}

DWORD MultiLock::all_lock(DWORD timeout)
{
	return lock(sync_semaphore_.max_lock_count(), timeout);
}

DWORD MultiLock::spin_lock(ULONG lock_cnt, DWORD timeout)
{
	const ULONGLONG start_tick = GetTickCount64();
	ULONGLONG cur_tick = start_tick;
	do
	{
		const DWORD ret = lock(lock_cnt, 0);
		if (ret == 0)
			break;

		cur_tick = GetTickCount64();

	} while (timeout != INFINITE && cur_tick - start_tick > timeout);

	return WAIT_TIMEOUT;
}

DWORD MultiLock::spin_all_lock(DWORD timeout)
{
	const ULONGLONG start_tick = GetTickCount64();
	ULONGLONG cur_tick = start_tick;
	do
	{
		const DWORD ret = all_lock(0);
		if (ret == 0)
			break;

		cur_tick = GetTickCount64();

	} while (timeout != INFINITE && cur_tick - start_tick > timeout);

	return WAIT_TIMEOUT;
}

DWORD MultiLock::unlock(ULONG unlock_cnt)
{
	const DWORD ret = sync_semaphore_.unlock(1);
	if (ret == 0)
	{
		assert(locked_cnt_ > unlock_cnt);
		locked_cnt_ -= unlock_cnt;
		return 0;
	}

	return ret;
}