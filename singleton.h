#pragma once
#include "stdafx.h"

class _singleton
{
private:
	std::map<size_t, PVOID64> singleton_by_types_;

public:
	template<typename _Ty>
	_Ty* getIntance()
	{
		_Ty* data = nullptr;
		size_t type = typeid(_Ty).hash_code();
		if (singleton_by_types_.find(type) != singleton_by_types_.end())
			data = static_cast<_Ty*>(singleton_by_types_[type]);

		if (data == nullptr)
			singleton_by_types_[type] = static_cast<PVOID64>(new _Ty);

		return static_cast<_Ty*>(singleton_by_types_[type]);
	}
};

static _singleton singleton;
#define Singleton(type) static_cast<type>(singleton.getIntance<type>());