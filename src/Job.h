#pragma once
#include "stdafx.h"
#include "SyncObject.h"

class JobBase abstract : public object
{
public:
	virtual bool RepeatWork() abstract;
};

template<typename _Ty>
using is_job = typename std::enable_if_t<std::is_base_of_v<JobBase, _Ty>, _Ty>*;

DEFINE_WRAPPER_HUB(JobBase);
DEFINE_WRAPPER_NODE(JobBase);