#include "stdafx.h"
#include "Wrapper.h"

namespace sync
{
	static const WORD default_semaphore_init_value = 0;
	static const WORD default_semaphore_limit_value = 300;

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
		Semaphore(LONG init_count = default_semaphore_init_value, LONG max_count = default_semaphore_limit_value)
			: SyncHandle(::CreateSemaphore(nullptr, init_count, max_count, nullptr)),
			init_count_(init_count), max_count_(max_count)
		{}

	private:
		LONG init_count_;
		LONG max_count_;
	};

	class Event : public SyncHandle
	{
	public:
		Event(BOOL is_menual_reset = default_event_is_menual_reset, BOOL init_state = default_event_init_state)
			: SyncHandle(::CreateEvent(nullptr, is_menual_reset, init_state, nullptr)),
			is_menual_reset_(is_menual_reset), init_state_(init_state)
		{}
	private:
		BOOL is_menual_reset_;
		BOOL init_state_;
	};
}