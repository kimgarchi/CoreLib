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

protected:
	friend class LockBase;
	friend class SingleLock;
	friend class MultiLock;
	friend class RWLock;

	friend class SyncStation;

	inline const HANDLE& handle() { return handle_; }

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
	LockBase();

	virtual bool Lock(DWORD timeout = INFINITE) { return _Lock(timeout); }
	virtual bool SpinLock(DWORD timeout = INFINITE) { return _SpinLock(timeout); }
	virtual bool Release() { return _Release(); }

protected:
	virtual bool _Lock(DWORD timeout) abstract;
	virtual bool _SpinLock(DWORD timeout) abstract;
	virtual bool _Release() abstract;

	std::atomic<bool>& signaled() { return signaled_; }

private:
	std::atomic<bool> signaled_;
};

class SingleLock : public LockBase
{
public:
	SingleLock(const SingleLock&) = delete;

	SingleLock(SyncMutexHub& hub, bool immediate_lock = true);
	SingleLock(SyncMutexNode& node, bool immediate_lock = true);

	virtual ~SingleLock();

private:

	virtual bool _Lock(DWORD timeout) override;
	virtual bool _SpinLock(DWORD timeout) override;
	virtual bool _Release() override;
	inline HANDLE handle() { return mutex_node_->handle(); }

	SyncMutexNode mutex_node_;
};

class MultiLock : public LockBase
{
public:
	MultiLock(const MultiLock&) = delete;

	MultiLock(SyncSemaphoreHub& hub, bool immediate_lock = true);
	MultiLock(SyncSemaphoreNode& node, bool immediate_lock = true);

	virtual ~MultiLock();

private:
	virtual bool _Lock(DWORD timeout) override;
	virtual bool _SpinLock(DWORD timeout) override;
	virtual bool _Release() override;
	inline HANDLE handle() { return semaphore_node_->handle(); }

	bool _Release(LONG& prev_count __out, LONG release_count = 1);

	SyncSemaphoreNode semaphore_node_;
};

class RWLock : public object
{
public:
	RWLock(SyncMutexHub& mutex_hub, SyncSemaphoreHub& semaphore_hub);
	RWLock(SyncMutexNode& mutex_node, SyncSemaphoreHub& semaphore_hub);
	RWLock(SyncMutexHub& mutex_hub, SyncSemaphoreNode& semaphore_node);
	RWLock(SyncMutexNode& mutex_node, SyncSemaphoreNode& semaphore_node);

	virtual ~RWLock();

	bool WriteLock(DWORD timeout = INFINITE);
	bool ReadLock(DWORD timeout = INFINITE);
	
	bool WriteRelease();
	bool ReadRelease();

private:
	friend class SyncStation;

	inline HANDLE write_handle() { return mutex_node_->handle(); }
	inline HANDLE read_handle() { return semaphore_node_->handle(); }

	SyncMutexNode mutex_node_;
	SyncSemaphoreNode semaphore_node_;

	std::atomic<LONG> signaled_count_;
};

DEFINE_WRAPPER_HUB(SingleLock);
DEFINE_WRAPPER_NODE(SingleLock);

DEFINE_WRAPPER_HUB(MultiLock);
DEFINE_WRAPPER_NODE(MultiLock);

DEFINE_WRAPPER_HUB(RWLock);
DEFINE_WRAPPER_NODE(RWLock);