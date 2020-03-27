#pragma once
#include "stdafx.h"


static const WORD default_semaphore_max_value = 100;
static const BOOL default_event_is_menual_reset = FALSE;
static const BOOL default_event_init_state = FALSE;

class SyncHandle abstract : public object
{
public:
	friend class SyncStation;

	SyncHandle(HANDLE handle)
		: handle_(handle)
	{
		assert(handle);
	}

	~SyncHandle()
	{
		CloseHandle(handle_);
	}

	const HANDLE handle() { return handle_; }

private:
	HANDLE handle_;
};

class SyncMutex : public SyncHandle
{
public:
	SyncMutex()
		: SyncHandle(::CreateMutex(nullptr, false, nullptr))
	{}

	inline bool Release() { return ReleaseMutex(handle()); }
};

class SyncSemaphore : public SyncHandle
{
public:
	SyncSemaphore(LONG init_count = default_semaphore_max_value, LONG max_count = default_semaphore_max_value)
		: SyncHandle(::CreateSemaphore(nullptr, init_count, max_count, nullptr)), prev_signal_count_(init_count), max_count_(max_count)
	{
		assert(init_count <= max_count);
	}

	bool Release(LONG& prev_count, LONG release_count = 1) 
	{
		std::unique_lock<std::mutex> lock(mtx_);
		if (prev_signal_count_ - release_count > max_count_)
		{
			assert(false);
			return false;
		}

		if (ReleaseSemaphore(handle(), release_count, &prev_count) == false)
		{
			assert(false);
			return false;
		}

		prev_signal_count_.store(prev_count);

		return true;
	}

	bool Release(LONG release_count = 1)
	{
		std::unique_lock<std::mutex> lock(mtx_);
		if (prev_signal_count_ - release_count > max_count_)
		{
			assert(false);
			return false;
		}

		LONG prev_count = 1;
		if (ReleaseSemaphore(handle(), release_count, &prev_count) == false)
		{
			assert(false);
			return false;
		}

		prev_signal_count_.store(prev_count);

		return true;
	}

private:
	std::mutex mtx_;
	std::atomic<LONG> prev_signal_count_;
	LONG max_count_;
};

class SyncEvent : public SyncHandle
{
public:
	SyncEvent(BOOL is_menual_reset = default_event_is_menual_reset, BOOL init_state = default_event_init_state)
		: SyncHandle(::CreateEvent(nullptr, is_menual_reset, init_state, nullptr)),
		is_menual_reset_(is_menual_reset), init_state_(init_state)
	{}

	inline bool Release() { return is_menual_reset_ == false ? SetEvent(handle()) : ResetEvent(handle()); }

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
