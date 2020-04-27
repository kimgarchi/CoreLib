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
	
	virtual SYNC_STATE state() abstract;
	
protected:
	friend class LockBase;
	friend class InnerLock;
	friend class SingleLock;
	friend class MultiLock;
	friend class RWLock;	

	inline const HANDLE handle() { return handle_; }
	DWORD Lock(DWORD timeout = INFINITE);
	virtual bool Release() abstract;

private:
	HANDLE handle_;
};

class SyncMutex : public SyncHandle
{
public:
	SyncMutex();
	SyncMutex(const SyncMutex& mutex);

	virtual ~SyncMutex();
	
	virtual bool Release() override { return ReleaseMutex(handle()); }
	virtual SYNC_STATE state() override;
};

class SyncSemaphore : public SyncHandle
{
public:
	SyncSemaphore(LONG max_count);
	SyncSemaphore(LONG init_count, LONG max_count);
	virtual ~SyncSemaphore();

	virtual bool Release() override { return _Release(); }
	virtual SYNC_STATE state() override;	
	LONG lock_count() { return current_count_.load(); }
	LONG max_count() { return max_count_; }

private:
	bool _Release(LONG& prev_count __out, LONG release_count = 1);
	bool _Release(LONG release_count = 1);

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

class LockBase abstract : public object
{
public:
	virtual DWORD Lock(DWORD timeout = INFINITE) { return _Lock(timeout); }
	virtual DWORD SpinLock(DWORD timeout = INFINITE) { return _SpinLock(timeout); }
	virtual bool Release() { return _Release(); }

	virtual SYNC_STATE state() abstract;

protected:
	virtual DWORD _Lock(DWORD timeout) abstract;
	virtual DWORD _SpinLock(DWORD timeout) abstract;
	virtual bool _Release() abstract;
};

class SingleLock : protected LockBase
{
public:
	SingleLock(SyncMutexHub& hub);
	SingleLock(SyncMutexNode& node);

	virtual ~SingleLock();

	virtual SYNC_STATE state() override { return mutex_node_->state(); }

protected:
	virtual DWORD _Lock(DWORD timeout) override { return mutex_node_->Lock(timeout); }
	virtual DWORD _SpinLock(DWORD timeout) override;
	virtual bool _Release() override;

	SyncMutexNode mutex_node_;
};

class MultiLock : public LockBase
{
public:
	MultiLock(SyncSemaphoreHub& hub);
	MultiLock(SyncSemaphoreNode& node);

	virtual ~MultiLock();
	
	virtual SYNC_STATE state() override { return semaphore_node_->state(); }

protected:
	virtual DWORD _Lock(DWORD timeout) override { return semaphore_node_->Lock(timeout); }
	virtual DWORD _SpinLock(DWORD timeout) override;
	virtual bool _Release() override;

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

	DWORD ReadLock(DWORD timeout = INFINITE);
	DWORD WriteLock(DWORD timeout = INFINITE);

	DWORD ReadSpinLock(DWORD timeout = INFINITE);
	DWORD WriteSpinLock(DWORD timeout = INFINITE);

private:
	virtual DWORD Lock(DWORD timeout) override { return WAIT_OBJECT_0; };
	virtual DWORD SpinLock(DWORD timeout) override { return WAIT_OBJECT_0; };
	virtual bool Release() { return true; };
};

DEFINE_WRAPPER_HUB(SingleLock);
DEFINE_WRAPPER_NODE(SingleLock);

DEFINE_WRAPPER_HUB(MultiLock);
DEFINE_WRAPPER_NODE(MultiLock);

DEFINE_WRAPPER_HUB(RWLock);
DEFINE_WRAPPER_NODE(RWLock);