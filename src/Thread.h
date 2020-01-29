#pragma once
#include "stdafx.h"
#include "JobStation.h"

class Thread : public object
{
public:
	friend class ThreadManager;

	Thread(JobHub job_hub);
	virtual ~Thread();

	bool Run();
	bool Stop();
	bool Join();

private:
	std::future 
	std::thread thread_;
	JobNode job_node_;
};