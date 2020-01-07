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

using MutexHub = wrapper_hub<Mutex>;
using SharedMutexHub = wrapper_hub<SharedMutex>;

using MutexNode = wrapper_node<Mutex>;
using SharedMutexNode = wrapper_node<SharedMutex>;

class BlockController final : public Singleton<BlockController>
{
private:	
	using Clasps = std::unordered_map<TypeID, MutexHub>;
	using ShrClasps = std::unordered_map<TypeID, SharedMutexHub>;
	using Lock = std::unique_lock<std::mutex>;

public:
	template<typename _Ty, is_object<_Ty> = nullptr>
	decltype(auto) ObtainClasp()
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
	decltype(auto) ObtainShrClasp()
	{
		Lock(mtx_);

		TypeID tid = typeid(_Ty).hash_code();
		if (shr_clasps_.find(tid) == shr_clasps_.end())
		{
			if (shr_clasps_.emplace(tid, make_wrapper_hub<Mutex>()).second)
				std::bad_alloc{};
		}

		return shr_clasps_.at(tid).make_node();
	}

private:
	Clasps clasps_;
	ShrClasps shr_clasps_;

	std::mutex mtx_;
};