#pragma once
#include "stdafx.h"
#include "Wrapper.h"

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

using SyncHub = wrapper_hub<SyncHandle>;
using SyncNode = wrapper_node<SyncHandle>;

class Mutex : public SyncHandle
{
public:
	Mutex()
		: SyncHandle(::CreateMutex(nullptr, false, nullptr))
	{}
};

class Semaphore : public SyncHandle
{
public:
	Semaphore(LONG init_count, LONG max_count)
		: SyncHandle(std::forward<HANDLE>(::CreateSemaphore(nullptr, init_count, max_count, nullptr))), 
		init_count_(init_count), max_count_(max_count)
	{}

private:	
	LONG init_count_;
	LONG max_count_;
};

class Event : public SyncHandle
{
public:
	Event(BOOL is_menual_reset, BOOL init_state)
		: SyncHandle(std::forward<HANDLE>(::CreateEvent(nullptr, is_menual_reset, init_state, nullptr))),
		is_menual_reset_(is_menual_reset), init_state_(init_state)
	{}
private:
	BOOL is_menual_reset_;
	BOOL init_state_;
};

using MutexHub = wrapper_hub<Mutex>;
using MutexNode = wrapper_node<Mutex>;
using MutexNodes = std::vector<MutexNode>;

class Lock
{
public:
	void* operator new(size_t) = delete;
	virtual bool WaitSignal() abstract;
};

class ExclusiveLock : public Lock
{
private:
	using Handles = std::vector<HANDLE>;

public:
	ExclusiveLock(MutexNodes&& mutex_nodes, DWORD dwMilliseconds = INFINITE)
		: mutex_nodes_(std::forward<MutexNodes>(mutex_nodes)), handle_count_(static_cast<DWORD>(mutex_nodes.size())), dwMilliseconds_(dwMilliseconds)
	{
		assert(mutex_nodes_.empty() == false);
		auto handle_count = mutex_nodes_.size();
		if (handle_count == 0)
			return;

		handles_.reserve(handle_count);
		std::for_each(mutex_nodes_.begin(), mutex_nodes_.end(),
			[&](MutexNode& node)
		{
			handles_.emplace_back(node->handle());
		});

		while (!WaitSignal());
	}

	~ExclusiveLock()
	{
		std::for_each(handles_.begin(), handles_.end(),
			[](HANDLE handle)
		{
			assert(ReleaseMutex(handle));
		});
	}

private:
	virtual bool WaitSignal() override
	{
		DWORD value = WaitForMultipleObjects(handle_count_, handles_.data(), true, dwMilliseconds_);
		if (handle_count_ == 1 &&
			(value == WAIT_OBJECT_0 || value == WAIT_ABANDONED_0))
		{
			return true;
		}
		else if (value > WAIT_OBJECT_0 && value < WAIT_OBJECT_0 + handle_count_)
		{
			if (value != WAIT_OBJECT_0 + handle_count_ - 1)
				return false;

			return true;
		}
		else if (value > WAIT_ABANDONED_0 && value < WAIT_ABANDONED_0 + handle_count_)
		{
			assert(false);
			handle_count_ -= (value - WAIT_ABANDONED_0);
			if (handle_count_ == 0)
				return false;

			if (value != WAIT_ABANDONED_0 + handle_count_ - 1)
				return false;

			return true;
		}
		else if (value == WAIT_TIMEOUT)
		{
			assert(dwMilliseconds_ == INFINITE);
			return true;
		}

		assert(false);
		return false;
	}

	MutexNodes mutex_nodes_;
	DWORD handle_count_;
	DWORD dwMilliseconds_;
	Handles handles_;
};