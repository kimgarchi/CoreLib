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