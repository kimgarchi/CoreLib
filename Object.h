#pragma once
#include "stdafx.h"

class object;
template<typename _Ty>
using is_object = std::enable_if<std::is_base_of<object, _Ty>::value, _Ty>;

template<typename _Ty>
using is_not_object = std::enable_if<!std::is_base_of<object, _Ty>::value, _Ty>;

template<typename _Ty, typename is_object<_Ty>::type*>
class ObjectPool;

class object abstract
{
public:
	virtual void initilize() abstract;

	void* operator new (size_t) = delete;
	void operator delete (void* ptr) = delete;
	
private:
	template<typename _Ty, typename is_object<_Ty>::type*>
	friend class ObjectPool;

	void* operator new (size_t size, void* ptr)
	{
		assert(ptr != nullptr);
		return ptr;
	}
};

