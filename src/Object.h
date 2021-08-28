#pragma once
#include "stdafx.h"

class object;

template<typename _Ty>
using is_object_base = typename std::enable_if_t<std::is_base_of_v<object, _Ty>, _Ty>*;

template<typename _Ty>
class wrapper;

class object abstract
{
public:
	object()
		: use_count_(0), node_count_(0)
	{}

	void* operator new (size_t) = delete;
	virtual const size_t priority_value() const { return 0; }	

protected:
	inline const std::mutex& mtx() { return mtx_; }

private:
	template<typename _Ty>
	friend class wrapper;

	template<typename _Ty, is_object_base<_Ty>>
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

	std::mutex mtx_;
};

