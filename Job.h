#pragma once
#include "stdafx.h"
#include <thread>
#include "Job.h"

using JobID = DWORD;
using LockMtx = std::mutex;
using JobFlowLock = std::unordered_map<JobID, LockMtx>;

struct Completion abstract
{
	using compe_job = std::function<void()>;
};

class JobBase abstract
{
public:
	JobBase()

protected:
	virtual bool update() abstract;

private:
	using Thread = std::thread;
	Thread worker_;
};