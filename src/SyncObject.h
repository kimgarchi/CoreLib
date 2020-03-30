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

	inline const HANDLE handle() { return handle_; }
	virtual SYNC_STATE state() abstract;

private:
	HANDLE handle_;
};

class SyncMutex : public SyncHandle
{
public:
	SyncMutex();
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

private:
	std::mutex mtx_;
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
DEFINE_WRAPPER_HUB(SyncSemaphore);
DEFINE_WRAPPER_HUB(SyncEvent);

DEFINE_WRAPPER_NODE(SyncMutex);
DEFINE_WRAPPER_NODE(SyncSemaphore);
DEFINE_WRAPPER_NODE(SyncEvent);
