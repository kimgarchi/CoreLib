#pragma once
#include "stdafx.h"

class object;

template<typename _Ty>
using is_object = std::enable_if<std::is_base_of<object, _Ty>::value, _Ty>;

template<typename _Ty, typename is_object<_Ty>::type*>
class ObjectPool;

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

	template<typename _Ty, typename is_object<_Ty>::type*>
	friend class ObjectPool;

	void* operator new (size_t size, void* ptr)
	{
		assert(ptr != nullptr);
		return ptr;
	}

	void operator delete (void* ptr) {}

	Count& use_cnt() { return use_count_; }
	Count& node_cnt() { return node_count_; }

	Count use_count_;
	Count node_count_;
};

