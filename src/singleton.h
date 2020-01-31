#pragma once
#include "stdafx.h"

template<typename _Ty>
class Singleton abstract
{
public:
	static _Ty& GetInstance()
	{
		static _Ty instance;
		return instance;
	}
};

template<typename _Ty>
using is_singleton = typename std::enable_if_t<std::is_base_of_v<Singleton<_Ty>, _Ty>, _Ty>*;