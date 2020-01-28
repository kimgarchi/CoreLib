#pragma once
#include "stdafx.h"
#include "singleton.h"
#include "Job.h"
#include "Wrapper.h"

using JobHub = wrapper_hub<Job>;
using JobNode = wrapper_node<Job>;

enum class JOB_TYPE
{
	READ,
	WRITE,
};

class JobStation : public Singleton<JobStation>
{
private:
	using AllocJobID = std::atomic_size_t;
	
public:
	template<typename ..._Tys>
	bool BookingJob(JOB_TYPE type, Job job)
	{
		std::unique_lock<std::mutex> lock(mtx_);
		auto types = TypeHarvest::GetInstance().harvest<_Tys...>();

		
		return true;
	}

private:	
	std::mutex mtx_;
};