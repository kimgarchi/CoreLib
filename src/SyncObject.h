#pragma once
#include "stdafx.h"

enum class SYNC_STATE
{
	UNLOCK,
	SEGMENT_LOCK,
	FULL_LOCK,
};

class SyncHandle abstract : public object
{
public:
	SyncHandle(HANDLE handle);
	virtual ~SyncHandle();
	
	virtual DWORD Lock(DWORD timeout = INFINITE) { return WaitForSingleObject(handle_, timeout); }
	virtual bool Release() abstract;

	inline const HANDLE handle() { return handle_; }

protected:
	friend class LockBase;
	friend class SingleLock;
	friend class MultiLock;
	friend class RWLock;
	friend class SyncStation;

private:
	HANDLE handle_;
};

class SyncMutex : public SyncHandle
{
public:
	SyncMutex(BOOL b_init = FALSE);
	SyncMutex(const SyncMutex& mutex);

	virtual bool Release() override { return ReleaseMutex(handle()); }
};

class SyncSemaphore : public SyncHandle
{
public:
	SyncSemaphore(LONG max_count);
	SyncSemaphore(LONG init_count, LONG max_count);
	SyncSemaphore(const SyncSemaphore& semaphore);

	virtual bool Release() override { return _Release(); }
	LONG max_count() { return max_count_; }

private:
	friend class MultiLock;
	friend class RWLock;

	bool _Release(LONG& prev_count __out, LONG release_count = 1);
	bool _Release(LONG release_count = 1);

	const LONG init_count_;
	const LONG max_count_;
};

class SyncEvent : public SyncHandle
{
public:
	SyncEvent(BOOL is_menual_reset = false, BOOL init_state = false);
	SyncEvent(const SyncEvent& event);
	
	virtual ~SyncEvent();
	
	virtual DWORD Lock(DWORD timeout = INFINITE) override;
	virtual bool Release() override { return SetEvent(handle()); }
	
private:
	BOOL is_menual_reset_;
	BOOL init_state_;
};

DEFINE_WRAPPER_HUB(SyncHandle);
DEFINE_WRAPPER_NODE(SyncHandle);

DEFINE_WRAPPER_HUB(SyncMutex);
DEFINE_WRAPPER_NODE(SyncMutex);

DEFINE_WRAPPER_HUB(SyncSemaphore);
DEFINE_WRAPPER_NODE(SyncSemaphore);

DEFINE_WRAPPER_HUB(SyncEvent);
DEFINE_WRAPPER_NODE(SyncEvent);

class LockBase abstract : public object
{
public:
	LockBase(const LockBase&) = delete;
	LockBase();
	virtual ~LockBase();

	virtual bool Lock(DWORD timeout = INFINITE);
	virtual bool SpinLock(DWORD timeout = INFINITE) { return _SpinLock(timeout); }
	virtual bool Release();

protected:
	virtual bool _Lock(DWORD timeout) abstract;
	virtual bool _SpinLock(DWORD timeout) abstract;
	virtual bool _Release() abstract;
	virtual bool _Destory() abstract;

	LONG signaled_count_;
};

class SingleLock : public LockBase
{
public:
	SingleLock(SyncMutex& sync_mutex, bool immediate_lock = true);
	virtual ~SingleLock();

protected:
	virtual bool _Lock(DWORD timeout) override;
	virtual bool _SpinLock(DWORD timeout) override;
	virtual bool _Release() override;
	virtual bool _Destory() override;
	
private:
	SyncMutex& sync_mutex_;
};

class MultiLock : public LockBase
{
public:
	MultiLock(SyncSemaphore& sync_semaphore, bool immediate_lock = true);
	MultiLock(const MultiLock&) = delete;
	virtual ~MultiLock();

protected:
	virtual bool _Lock(DWORD timeout) override;
	virtual bool _SpinLock(DWORD timeout) override;
	virtual bool _Release() override;
	virtual bool _Destory() override;

	bool _Release(LONG& prev_count __out, LONG release_count = 1);
	LONG max_count() { return sync_semaphore_.max_count(); }

private:
	SyncSemaphore& sync_semaphore_;
};

class RWLock : public MultiLock
{
public:
	RWLock(SyncSemaphore& sync_semaphore);
	RWLock(const RWLock&) = delete;

	virtual ~RWLock();

	bool WriteLock(DWORD timeout = INFINITE);
	bool ReadLock(DWORD timeout = INFINITE);
	
	bool WriteRelease();
	bool ReadRelease();

private:
	friend class SyncStation;
};