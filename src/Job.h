#pragma once
#include "stdafx.h"

class JobBase abstract : public object
{
public:
	virtual bool Prepare() abstract;
	virtual bool RepeatWork() abstract;
};

template<typename _Ty>
using is_job = typename std::enable_if_t<std::is_base_of_v<JobBase, _Ty>, _Ty>*;

using JobHub = wrapper_hub<JobBase>;
using JobNode = wrapper_node<JobBase>;