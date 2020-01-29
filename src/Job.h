#pragma once
#include "stdafx.h"
#include "Object.h"

using JobID = size_t;
using Func = std::function<void()>;

class Job : public object
{
public:
	Job(Func func)
		: func_(func), job_id_(INVALID_JOB_ID), begin_thread_id_(GetCurrentThreadId())
	{}

	virtual ~Job()
	{
		assert(job_id_ != INVALID_JOB_ID);
	}

	bool Run(JobID job_id)
	{
		job_id_ = job_id;
		if (job_id_ != INVALID_JOB_ID)
		{
			assert(false);
			return false;
		}

		if (begin_thread_id_ != GetCurrentThreadId())
		{
			assert(false);
			return false;
		}	

		func_();

		return true;
	}

private:
	friend class JobStation;

	JobID job_id_;
	DWORD begin_thread_id_;
	Func func_;
};