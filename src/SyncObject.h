#pragma once
#include "stdafx.h"
#include "Wrapper.h"

class SyncHandle final : public object
{
	SyncHandle(HANDLE&& handle)
		: handle_(handle) 
	{
		assert(handle);
	}

	~SyncHandle()
	{
		CloseHandle(handle_);
	}

private:
	HANDLE handle_;
};

class SyncObject abstract
{
public:
	SyncObject(HANDLE&& handle)
		: sync_handle_(make_wrapper_hub<SyncHandle>(handle))
	{
		assert(handle != nullptr);
	}

	virtual ~SyncObject()
	{
		assert(wait_count() == 0);
	}

	decltype(auto) node() { return sync_handle_.make_node(); }

	virtual WORD wait_count()
	{
		const auto& node_count = sync_handle_.node_count();
		if (node_count > 0)
			return node_count - 1;

		return 0;
	}

private:	
	wrapper_hub<SyncHandle> sync_handle_;
};

class Mutex : public SyncObject
{
public:
	Mutex()
		: SyncObject(std::forward<HANDLE>(CreateMutex(nullptr, false, nullptr)))
	{}
};

class Semaphore : public SyncObject
{
public:
	Semaphore(LONG init_count, LONG max_count)
		: SyncObject(std::forward<HANDLE>(CreateSemaphore(nullptr, init_count, max_count, nullptr)))
	{}
};

class Event : public SyncObject
{
public:
	Event(BOOL is_menual_reset, BOOL init_state)
		: SyncObject(std::forward<HANDLE>(CreateEvent(nullptr, is_menual_reset, init_state, nullptr)))
	{}
};