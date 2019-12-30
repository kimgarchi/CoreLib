#pragma once
#include "stdafx.h"
#include "singleton.h"
#include "ObjectPool.h"

const static size_t _default_obj_alloc_cnt_ = 10;

class ObjectStation final : public Singleton<ObjectStation>
{
public:
	~ObjectStation()
	{
		try
		{
			std::for_each(object_pool_by_tid_.begin(), object_pool_by_tid_.end(),
				[](std::map<size_t, ObjectPoolBase*>::value_type& value)
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
	bool BindObjectPool(DWORD obj_alloc_cnt = _default_obj_alloc_cnt_)
	{
		ObjectPool<_Ty>* object_pool = new ObjectPool<_Ty>(obj_alloc_cnt);
		return object_pool_by_tid_.insert(ObjectPoolByTid::value_type(typeid(_Ty).hash_code(), object_pool)).second;
	}

	template<typename _Ty>
	_Ty* Pop()
	{
		auto itor = object_pool_by_tid_.find(typeid(_Ty).hash_code());
		if (itor == object_pool_by_tid_.end())
			return nullptr;

		ObjectPool<_Ty>* object_pool = static_cast<ObjectPool<_Ty>*>(itor->second);
		return object_pool->Pop();
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
	using ObjectPoolByTid = std::unordered_map<size_t, ObjectPoolBase*>;
	ObjectPoolByTid object_pool_by_tid_;
};