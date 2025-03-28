#pragma once
#include "SyncObject.h"

class LockBase abstract
{
public:
	LockBase();
	virtual ~LockBase();

	LockBase(const LockBase&) = delete;	
	LockBase& operator=(LockBase&) = delete;
	
protected:		
	ULONG locked_cnt_;
};

class SingleLock : public LockBase
{
public:
	SingleLock(SyncMutex& sync_mutex, bool immediate_lock = true, DWORD timeout = INFINITE);
	virtual ~SingleLock();

	DWORD lock(DWORD timeout);
	DWORD spin_lock(DWORD timeout);
	DWORD unlock();

private:
	SyncMutex& sync_mutex_;
};

class MultiLock : public LockBase
{
public:
	MultiLock(SyncSemaphore& sync_semaphore, bool immediate_lock = true, ULONG lock_cnt = 1, DWORD timeout = INFINITE);
	virtual ~MultiLock();

	DWORD lock(ULONG lock_cnt, DWORD timeout);
	DWORD spin_lock(ULONG lock_cnt, DWORD timeout);
	DWORD all_lock(DWORD timeout);	
	DWORD spin_all_lock(DWORD timeout);
	DWORD unlock(ULONG unlock_cnt);

private:
	SyncSemaphore& sync_semaphore_;
};