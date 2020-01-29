#include "stdafx.h"
#include "Thread.h"

Thread::Thread(JobHub job_hub)
	: job_node_(job_hub.make_node())
{
}

Thread::~Thread()
{
	//thread_
}

bool Thread::Run()
{
	std::promise<bool>

	return false;
}

bool Thread::Stop()
{
	if (thread_.joinable() == false)
		return false;



	return true;
}

bool Thread::Join()
{
	return false;
}
