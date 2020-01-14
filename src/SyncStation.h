#pragma once
#include "stdafx.h"
#include "singleton.h"
#include "SyncObject.h"

using MutexHub = wrapper_hub<sync::Mutex>;
using MutexNode = wrapper_node<sync::Mutex>;

using SemaphoreHub = wrapper_hub<sync::Semaphore>;
using SemaphoreNode = wrapper_hub<sync::Semaphore>;

enum class HANDLE_STATE
{
	NONE,
	IDLE,
	READ_LOCK,
	WRITE_LOCK,
};

class SyncStation : public Singleton<SyncStation>
{
private:
	friend class BlockController;

	class RWHandle
	{
	public:
		RWHandle(WORD idx)
			: idx_(idx), mutex_(make_wrapper_hub<sync::Mutex>()), semaphore_(make_wrapper_hub<sync::Semaphore>())
		{}

		decltype(auto) state()
		{
			DWORD ret = WaitForSingleObject(mutex_.get(), 0);
			if (ret == WAIT_OBJECT_0)
				return HANDLE_STATE::WRITE_LOCK;

			ret = WaitForSingleObject(semaphore_.get(), 0);
			if (ret == WAIT_OBJECT_0)
				return HANDLE_STATE::READ_LOCK;

			return HANDLE_STATE::IDLE;
		}

		decltype(auto) WriteHandle() { return mutex_.get(); }
		decltype(auto) Readhandle() { return semaphore_.get(); }

	private:
		WORD idx_;
		MutexHub mutex_;
		SemaphoreHub semaphore_;
	};

	using HandleByType = std::unordered_map<TypeID, RWHandle>;
	using Handles = std::vector<HANDLE>;
	using Types = std::vector<TypeID>;

	template<typename _Ty>
	bool RecordHandle()
	{
		std::unique_lock<std::mutex>(mtx_);
		auto tid = typeid(_Ty).hash_code();

		if (handle_by_type_.find(tid) == handle_by_type_.end())
			return false;

		auto idx = handle_by_type_.size();
		assert(handle_by_type_.size() == write_handles_.size() && write_handles_.size() == read_handles_.size());

		auto ret = handle_by_type_.emplace(tid, idx);
		auto push_result = ret.second;
		auto rw_handle = ret.first->second;
		
		if (push_result == false)
		{
			assert(false);
			return false;
		}

		write_handles_.emplace_back(rw_handle.WriteHandle());
		read_handles_.emplace_back(rw_handle.Readhandle());
		
		return true;
	}

	template<typename _Ty>
	HANDLE_STATE handle_state()
	{
		std::unique_lock<std::mutex>(mtx_);
		auto tid = typeid(_Ty).hash_code();

		auto itor = handle_by_type_.find(tid);
		if (itor == handle_by_type_.end())
		{
			assert(false);
			return HANDLE_STATE::NONE;
		}
			

		return itor->second.state();
	}
	
	inline decltype(auto) ReadHandles() { return read_handles_.data(); }
	inline decltype(auto) WriteHandles() { return write_handles_.data(); }

	HandleByType handle_by_type_;
	Handles read_handles_;
	Handles write_handles_;
	Types types_;

	std::mutex mtx_;
};