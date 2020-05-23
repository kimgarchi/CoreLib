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
	
	inline decltype(auto) Lock(DWORD timeout = INFINITE) { return WaitForSingleObject(handle_, timeout); }
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
	
	inline bool Release() { return is_menual_reset_ == false ? SetEvent(handle()) : ResetEvent(handle()); }
	
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
	virtual bool Lock(DWORD timeout = INFINITE) { return _Lock(timeout); }
	virtual bool SpinLock(DWORD timeout = INFINITE) { return _SpinLock(timeout); }
	virtual bool Release() { return _Release(); }

protected:
	virtual bool _Lock(DWORD timeout) abstract;
	virtual bool _SpinLock(DWORD timeout) abstract;
	virtual bool _Release() abstract;
};

class SingleLock : public LockBase
{
public:
	SingleLock(const SingleLock&) = delete;

	SingleLock(SyncMutexHub& hub, bool immediate_lock = true);
	SingleLock(SyncMutexNode& node, bool immediate_lock = true);

	virtual ~SingleLock();

private:
	friend class RWLock;

	virtual bool _Lock(DWORD timeout) override;
	virtual bool _SpinLock(DWORD timeout) override;
	virtual bool _Release() override;
	inline HANDLE handle() { return mutex_node_->handle(); }

	SyncMutexNode mutex_node_;
	std::atomic<bool> signaled_;
};

class MultiLock : public LockBase
{
public:
	MultiLock(const MultiLock&) = delete;

	MultiLock(SyncSemaphoreHub& hub, bool immediate_lock = true);
	MultiLock(SyncSemaphoreNode& node, bool immediate_lock = true);

	virtual ~MultiLock();

private:
	friend class RWLock;

	virtual bool _Lock(DWORD timeout) override;
	virtual bool _SpinLock(DWORD timeout) override;
	virtual bool _Release() override;
	inline HANDLE handle() { return semaphore_node_->handle(); }

	SyncSemaphoreNode semaphore_node_;
	std::atomic<bool> signaled_;
};

class RWLock : public LockBase
{
public:
	RWLock(SyncMutexHub& mutex_hub, SyncSemaphoreHub& semaphore_hub);
	RWLock(SyncMutexNode& mutex_node, SyncSemaphoreHub& semaphore_hub);
	RWLock(SyncMutexHub& mutex_hub, SyncSemaphoreNode& semaphore_node);
	RWLock(SyncMutexNode& mutex_node, SyncSemaphoreNode& semaphore_node);

	bool WriteLock(DWORD timeout = INFINITE);
	bool ReadLock(DWORD timeout = INFINITE);
	
	bool WriteSpinLock(DWORD timeout = INFINITE);
	bool ReadSpinLock(DWORD timeout = INFINITE);
	
	inline bool WriteRelease() { return single_lock_.Release(); }
	inline bool ReadRelease() { return multi_lock_.Release(); }

private:
	friend class SyncStation;

	inline HANDLE write_handle() { return single_lock_.handle(); }
	inline HANDLE read_handle() { return multi_lock_.handle(); }

	virtual bool _Lock(DWORD timeout) override { return single_lock_.Lock(timeout); }
	virtual bool _SpinLock(DWORD timeout) override { return single_lock_.SpinLock(timeout); }
	virtual bool _Release() { return single_lock_.Release(); }

	SingleLock single_lock_;
	MultiLock multi_lock_;
};

DEFINE_WRAPPER_HUB(SingleLock);
DEFINE_WRAPPER_NODE(SingleLock);

DEFINE_WRAPPER_HUB(MultiLock);
DEFINE_WRAPPER_NODE(MultiLock);

DEFINE_WRAPPER_HUB(RWLock);
DEFINE_WRAPPER_NODE(RWLock);