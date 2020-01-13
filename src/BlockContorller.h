#pragma once
#include "stdafx.h"
#include "singleton.h"
#include "Wrapper.h"

struct Mutex : public object
{
	std::mutex mtx_;
};

struct SharedMutex : public object
{
	std::shared_mutex shr_mtx_;
};

using ExclusiveMutexHub = wrapper_hub<Mutex>;
using SharedMutexHub = wrapper_hub<SharedMutex>;

using ExclusiveMutexNode = wrapper_node<Mutex>;
using SharedMutexNode = wrapper_node<SharedMutex>;

class BlockController final : public Singleton<BlockController>
{
private:
	using Clasps = std::unordered_map<TypeID, ExclusiveMutexHub>;
	using SharedClasps = std::unordered_map<TypeID, SharedMutexHub>;
	using Lock = std::unique_lock<std::mutex>;

public:
	BlockController()
		: lock_keeper_hub_(make_wrapper_hub<Mutex>())
	{	
	}

	template<typename _Ty, is_object<_Ty> = nullptr>
	decltype(auto) ObtainExclusiveClasp()
	{
		Lock(mtx_);

		TypeID tid = typeid(_Ty).hash_code();
		if (clasps_.find(tid) == clasps_.end())
		{
			if (clasps_.emplace(tid, make_wrapper_hub<Mutex>()).second)
				std::bad_alloc{};
		}

		return clasps_.at(tid).make_node();
	}

	template<typename _Ty, is_object<_Ty> = nullptr>
	decltype(auto) ObtainSharedClasp()
	{
		Lock(mtx_);

		TypeID tid = typeid(_Ty).hash_code();
		if (shared_clasps_.find(tid) == shared_clasps_.end())
		{
			if (shared_clasps_.emplace(tid, make_wrapper_hub<Mutex>()).second)
				std::bad_alloc{};
		}

		return shared_clasps_.at(tid).make_node();
	}

private:
	Clasps clasps_;
	SharedClasps shared_clasps_;

	Mutex exclusive_clasp_mtx_;
	Mutex shared_clasp_mtx_;
	ExclusiveMutexHub lock_keeper_hub_;
};