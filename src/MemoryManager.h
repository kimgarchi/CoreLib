#pragma once
#include "stdafx.h"
#include "ObjectPool.h"

#ifdef _DEBUG
const static size_t _default_mem_obj_mul_cnt_ = 100;
const static float _default_mem_obj_mul_extend_remain_rate = 30.0f;
#else
const static size_t _default_mem_obj_mul_cnt_ = 1000;
const static float _default_mem_obj_mul_extend_remain_rate = 50.0f;
#endif

class MemoryManager final
{
private:
	using MemoryPoolByTid = std::unordered_map<TypeID, MemoryPoolBase*>;

public:
	MemoryManager()
	{
		ULONG HeapInformationValue = 2;

		if (HeapSetInformation(GetProcessHeap(), 
				HeapCompatibilityInformation, 
				&HeapInformationValue, 
				sizeof(HeapInformationValue)) == false)
		{
			assert(false);
			throw std::invalid_argument("heap set information failed");
		}
	}

	~MemoryManager()
	{
		try
		{
			std::for_each(memory_pool_by_tid_.begin(), memory_pool_by_tid_.end(),
				[](std::map<TypeID, MemoryPoolBase*>::value_type& value)
			{
				auto ptr = value.second;
				ptr->Clear();
				delete value.second;
			});
		}
		catch (std::bad_function_call & excp)
		{
			//... temp
			std::cout << excp.what() << std::endl;
			return;
		}

		memory_pool_by_tid_.clear();
	}



private:
	template<typename _Ty>
	bool IsBinding()
	{
		if (memory_pool_by_tid_.find(typeid(_Ty).hash_code()) == memory_pool_by_tid_.end())
			return false;

		return true;
	}

	template<typename _Ty>
	bool BindObjectPool(size_t obj_alloc_cnt = _default_mem_obj_mul_cnt_, float obj_extend_remain_rate = _default_mem_obj_mul_extend_remain_rate)
	{
		auto tid = typeid(_Ty).hash_code();
		return memory_pool_by_tid_.emplace(tid, new ObjectPool<_Ty>(obj_alloc_cnt, obj_extend_remain_rate)).second;
	}

	template<typename _Ty, typename ..._Tys>
	_Ty* Pop(_Tys&&... Args)
	{
		auto itor = memory_pool_by_tid_.find(typeid(_Ty).hash_code());
		if (itor == memory_pool_by_tid_.end())
		{
			assert(false);
			return nullptr;
		}

		ObjectPool<_Ty>* memory_pool = static_cast<ObjectPool<_Ty>*>(itor->second);
		return memory_pool->Pop(Args...);
	}

	template<typename _Ty>
	_Ty* Pop()
	{
		auto itor = memory_pool_by_tid_.find(typeid(_Ty).hash_code());
		if (itor == memory_pool_by_tid_.end())
		{
			assert(false);
			return nullptr;
		}

		ObjectPool<_Ty>* memory_pool = static_cast<ObjectPool<_Ty>*>(itor->second);
		return memory_pool->Pop();
	}

	template<typename _Ty>
	bool Push(_Ty*& object, TypeID type_id)
	{

		if (memory_pool_by_tid_.find(type_id) == memory_pool_by_tid_.end())
			return false;

		MemoryPoolBase* object_pool = memory_pool_by_tid_.at(type_id);
		return object_pool->Push(object);
	}


	MemoryPoolByTid memory_pool_by_tid_;
};