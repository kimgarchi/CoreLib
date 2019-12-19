#pragma once
#include "stdafx.h"

class object abstract
{
public:
	virtual void initilize() abstract;
};

template<typename _Ty>
using is_object = std::enable_if<std::is_base_of<object, _Ty>::value, _Ty>;