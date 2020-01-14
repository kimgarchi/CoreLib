#pragma once
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
		Event(BOOL is_menual_reset = default_event_is_menual_reset, BOOL init_state = default_event_init_state)
			: SyncHandle(std::forward<HANDLE>(::CreateEvent(nullptr, is_menual_reset, init_state, nullptr))),
			is_menual_reset_(is_menual_reset), init_state_(init_state)
		{}
	private:
		BOOL is_menual_reset_;
		BOOL init_state_;
	};

	template<typename ..._Tys>
	class TypeHarvest
	{
	protected:
		using HarvestTypes = std::vector<TypeID>;
		using Types = std::set<TypeID>;

	public:
		TypeHarvest()
		{
			Types tids;
			RecordLockType<_Tys...>(tids);
		}

		decltype(auto) harvest_types() { return harvest_types_; }

	private:
		template <typename _Ty>
		void RecordLockType(Types& tids)
		{
			tids.emplace(typeid(_Ty).hash_code());
			std::copy(tids.begin(), tids.end(), std::back_inserter(harvest_types_));
		}

		template<typename _fTy, typename _sTy, typename ..._Tys>
		void RecordLockType(Types& tids)
		{
			tids.emplace(typeid(_fTy).hash_code());
			RecordLockType<_sTy, _Tys...>();
		}

		HarvestTypes harvest_types_;
	};
}