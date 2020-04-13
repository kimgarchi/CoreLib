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
	friend class SingleLock;
	friend class MultiLock;

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

DEFINE_WRAPPER_HUB(SyncMutex);
DEFINE_WRAPPER_NODE(SyncMutex);

DEFINE_WRAPPER_HUB(SyncSemaphore);
DEFINE_WRAPPER_NODE(SyncSemaphore);

DEFINE_WRAPPER_HUB(SyncEvent);
DEFINE_WRAPPER_NODE(SyncEvent);

class LockBase abstract : public object
{
public:
	virtual DWORD Lock(DWORD timeout) abstract;
	virtual bool Release() abstract;
	virtual SYNC_STATE state() abstract;
};

class SingleLock : public LockBase
{
public:
	SingleLock(SyncMutexHub& hub, bool immedidate_lock = true);
	SingleLock(SyncMutexNode& node, bool immedidate_lock = true);

	virtual ~SingleLock();

	virtual DWORD Lock(DWORD timeout = INFINITE) override { return mutex_node_->Lock(timeout); }
	virtual bool Release() override;
	virtual SYNC_STATE state() override { return mutex_node_->state(); }

private:
	SyncMutexNode mutex_node_;
};

class MultiLock : public LockBase
{
public:
	MultiLock(SyncSemaphoreHub& hub, bool immedidate_lock = true);
	MultiLock(SyncSemaphoreNode& node, bool immedidate_lock = true);

	virtual ~MultiLock();

	virtual DWORD Lock(DWORD timeout = INFINITE) override { return semaphore_node_->Lock(timeout); }
	virtual bool Release() override;
	virtual SYNC_STATE state() override { return semaphore_node_->state(); }

private:	
	SyncSemaphoreNode semaphore_node_;
};

class RWLock : public SingleLock, MultiLock
{
public:
	RWLock(SyncMutexHub& mutex_hub, SyncSemaphoreHub& semaphore_hub);
	RWLock(SyncMutexNode& mutex_node, SyncSemaphoreHub& semaphore_hub);
	RWLock(SyncMutexHub& mutex_hub, SyncSemaphoreNode& semaphore_node);
	RWLock(SyncMutexNode& mutex_node, SyncSemaphoreNode& semaphore_node);

	virtual SYNC_STATE state() override;

	bool ReadLock(DWORD timeout = INFINITE);
	bool WriteLock(DWORD timeout = INFINITE);
};

DEFINE_WRAPPER_HUB(SingleLock);
DEFINE_WRAPPER_NODE(SingleLock);

DEFINE_WRAPPER_HUB(MultiLock);
DEFINE_WRAPPER_NODE(MultiLock);