#pragma once
#include "stdafx.h"

class object;

template<typename _Ty>
using is_object = typename std::enable_if_t<std::is_base_of_v<object, _Ty>, _Ty>*;

template<typename _Ty>
class wrapper;

using Count = std::atomic_size_t;
using AllocID = size_t;
using TypeID = size_t;

class object abstract
{
public:
	object()
		: use_count_(0), node_count_(0)
	{}

	void* operator new (size_t) = delete;
	
private:
	template<typename _Ty>
	friend class wrapper;

	template<typename _Ty, is_object<_Ty> = nullptr>
	friend class ObjectPool;

	void* operator new (size_t size, void* ptr)
	{
		assert(ptr != nullptr);
		return ptr;
	}

	void operator delete (void* ptr) {}

	Count& use_count() { return use_count_; }
	Count& node_count() { return node_count_; }

	Count use_count_;
	Count node_count_;
};

