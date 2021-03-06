#pragma once
#include "stdafx.h"
#include "SyncObject.h"

enum class JOB_TYPE
{
	READ,
	WRITE
};

class JobBase abstract : public object
{
public:
	virtual bool Execute() abstract;

protected:
	//virtual bool commit() abstract;
	//virtual bool rollback() abstract;
};

template<typename _Ty>
using is_job = typename std::enable_if_t<std::is_base_of_v<JobBase, _Ty>, _Ty>*;

DEFINE_WRAPPER_HUB(JobBase);
DEFINE_WRAPPER_NODE(JobBase);