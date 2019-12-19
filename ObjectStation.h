#pragma once
#include "stdafx.h"
#include "ObjectPool.h"

class ObjectStation final
{
public:
	~ObjectStation()
	{
		std::for_each(object_pool_by_tid_.begin(), object_pool_by_tid_.end(),
			[](std::map<size_t, ObjectPoolBase*>::value_type& value)
		{
			auto ptr = value.second;
			ptr->Clear();
			delete value.second;
		});

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
	bool BindObjectPool(DWORD init_count = 0, DWORD max_count = 0, DWORD extend_count = 0)
	{
		ObjectPool<_Ty>* object_pool = new ObjectPool<_Ty>(init_count, max_count, extend_count);
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

	template<typename _Ty>
	bool Push(_Ty*& object)
	{
		auto itor = object_pool_by_tid_.find(typeid(_Ty).hash_code());
		if (itor == object_pool_by_tid_.end())
			return false;

		ObjectPool<_Ty>* object_pool = static_cast<ObjectPool<_Ty>*>(itor->second);
		return object_pool->Push(object);
	}

private:
	using ObjectPoolByTid = std::map<size_t, ObjectPoolBase*>;
	ObjectPoolByTid object_pool_by_tid_;
};