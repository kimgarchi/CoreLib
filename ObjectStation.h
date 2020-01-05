#pragma once
#include "stdafx.h"
#include "singleton.h"
#include "ObjectPool.h"

#ifdef _DEBUG
const static size_t _default_obj_alloc_cnt_ = 100;
const static float _default_obj_extend_remain_rate = 30.0f;
#else
const static size_t _default_obj_alloc_cnt_ = 1000;
const static float _default_obj_extend_remain_rate = 50.0f;
#endif

class ObjectStation final : public Singleton<ObjectStation>
{
private:
	using ObjectPoolByTid = std::unordered_map<TypeID, ObjectPoolBase*>;

public:
	~ObjectStation()
	{
		try
		{
			std::for_each(object_pool_by_tid_.begin(), object_pool_by_tid_.end(),
				[](std::map<TypeID, ObjectPoolBase*>::value_type& value)
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

		object_pool_by_tid_.clear();		
	}

	template<typename _Ty>
	bool IsBinding()
	{
		if (object_pool_by_tid_.find(typeid(_Ty).hash_code()) == object_pool_by_tid_.end())
			return false;

		return true;
	}

	template<typename _Ty>
	bool BindObjectPool(size_t obj_alloc_cnt = _default_obj_alloc_cnt_, float obj_extend_remain_rate = _default_obj_extend_remain_rate)
	{
		auto tid = typeid(_Ty).hash_code();
		return object_pool_by_tid_.emplace(tid, new ObjectPool<_Ty>(obj_alloc_cnt, obj_extend_remain_rate)).second;
	}

	template<typename _Ty, typename ..._Tys>
	_Ty* Pop(_Tys&&... Args)
	{
		auto itor = object_pool_by_tid_.find(typeid(_Ty).hash_code());
		if (itor == object_pool_by_tid_.end())
		{
			assert(false);
			return nullptr;
		}

		ObjectPool<_Ty>* object_pool = static_cast<ObjectPool<_Ty>*>(itor->second);
		return object_pool->Pop(Args...);
	}

	template<typename _Ty, typename is_object<_Ty>::type * = nullptr>
	bool Push(_Ty*& object)
	{
		auto itor = object_pool_by_tid_.find(typeid(_Ty).hash_code());
		if (itor == object_pool_by_tid_.end())
			return false;

		ObjectPool<_Ty>* object_pool = static_cast<ObjectPool<_Ty>*>(itor->second);

		return object_pool->Push(object);
	}

private:
	ObjectPoolByTid object_pool_by_tid_;
};