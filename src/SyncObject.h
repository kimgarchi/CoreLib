#pragma once
#include "stdafx.h"

enum class SYNC_STATE
{
	UNLOCK,
	SEGMENT_LOCK,
	FULL_LOCK,
	MAX,
};

class SyncHandle abstract : public object
{
public:
	SyncHandle(HANDLE handle);
	virtual ~SyncHandle();
	
	virtual SYNC_STATE state() abstract;
	
protected:
	friend class LockBase;
	friend class SingleLock;
	friend class MultiLock;
	friend class RWLock;

	inline const HANDLE handle() { return handle_; }
	DWORD Lock(DWORD timeout = INFINITE);

private:
	HANDLE handle_;
};

class SyncMutex : public SyncHandle
{
public:
	SyncMutex();
	SyncMutex(const SyncMutex& mutex);

	virtual ~SyncMutex();
	
	inline bool Release() { return ReleaseMutex(handle()); }
	virtual SYNC_STATE state() override;
};

class SyncSemaphore : public SyncHandle
{
public:
	SyncSemaphore(LONG max_count);
	SyncSemaphore(LONG init_count, LONG max_count);
	virtual ~SyncSemaphore();

	bool Release(LONG& prev_count __out, LONG release_count = 1);
	bool Release(LONG release_count = 1);
	
	virtual SYNC_STATE state() override;	
	LONG lock_count() { return current_count_.load(); }

	LONG max_count() { return max_count_; }

private:
	std::atomic<LONG> current_count_;
	const LONG max_count_;
};

class SyncEvent : public SyncHandle
{
public:
	SyncEvent(BOOL is_menual_reset = false, BOOL init_state = false);
	virtual ~SyncEvent();
	
	inline bool Release() { return is_menual_reset_ == false ? SetEvent(handle()) : ResetEvent(handle()); }
	virtual SYNC_STATE state() override;

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

class InnerLock : public SyncMutex
{
public:
	InnerLock(const SyncMutex& sync_mutex, DWORD timeout = INFINITE);
	virtual ~InnerLock();

	inline DWORD remain_wait_seconds() { return remain_wait_seconds_; }
	SYNC_STATE state() { return SyncMutex::state(); }

private:
	DWORD remain_wait_seconds_;
};

class LockBase abstract : public object
{
public:
	DWORD Lock(DWORD timeout = INFINITE);
	DWORD SpinLock(DWORD timeout = INFINITE);

	bool Release();
	
protected:
	virtual DWORD _Lock(DWORD timeout) abstract;
	virtual DWORD _SpinLock(DWORD timeout) abstract;
	virtual bool _Release() abstract;
	virtual SYNC_STATE state() abstract;

private:
	SyncMutex sync_mutex_;
};

class SingleLock : protected LockBase
{
public:
	SingleLock(SyncMutexHub& hub);
	SingleLock(SyncMutexNode& node);

	virtual ~SingleLock();

protected:
	virtual DWORD _Lock(DWORD timeout) override;
	virtual DWORD _SpinLock(DWORD timeout) override;
	virtual bool _Release() override;
	virtual SYNC_STATE state() override { return mutex_node_->state(); }

	SyncMutex sync_mutex_;
	SyncMutexNode mutex_node_;
};

class MultiLock : public LockBase
{
public:
	MultiLock(SyncSemaphoreHub& hub);
	MultiLock(SyncSemaphoreNode& node);

	virtual ~MultiLock();
	
protected:
	virtual DWORD _Lock(DWORD timeout = INFINITE) override;
	virtual DWORD _SpinLock(DWORD timeout = INFINITE) override;
	virtual bool _Release() override;
	virtual SYNC_STATE state() override { return semaphore_node_->state(); }

	SyncSemaphore sync_semaphore_;
	SyncSemaphoreNode semaphore_node_;	
};

class RWLock : public SingleLock, MultiLock
{
private:
	enum class LOCK_TYPE
	{
		READ,
		WRITE,
		MAX,
	};

public:
	RWLock(SyncMutexHub& mutex_hub, SyncSemaphoreHub& semaphore_hub);
	RWLock(SyncMutexNode& mutex_node, SyncSemaphoreHub& semaphore_hub);
	RWLock(SyncMutexHub& mutex_hub, SyncSemaphoreNode& semaphore_node);
	RWLock(SyncMutexNode& mutex_node, SyncSemaphoreNode& semaphore_node);

	virtual SYNC_STATE state() override;

	bool ReadLock(DWORD timeout = INFINITE);
	bool WriteLock(DWORD timeout = INFINITE);

	bool ReadSpinLock(DWORD timeout = INFINITE);
	bool WriteSpinLock(DWORD timeout = INFINITE);

private:
	LOCK_TYPE lock_type_;
};

DEFINE_WRAPPER_HUB(SingleLock);
DEFINE_WRAPPER_NODE(SingleLock);

DEFINE_WRAPPER_HUB(MultiLock);
DEFINE_WRAPPER_NODE(MultiLock);