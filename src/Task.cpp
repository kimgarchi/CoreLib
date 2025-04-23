#include "stdafx.h"
#include "Task.h"
//#include "MemoryManager.h"
/*
Task::Task(std::size_t thread_count, std::shared_ptr<JobBase> job_ptr)
	: job_ptr_(job_ptr), is_runable_(true)
{
	assert(job_ptr_);
	//thread_que_ = allocate_deque<std::shared_ptr<Thread>>();
	Attach(thread_count);
}

Task::~Task()
{
	assert(Stop());
	thread_que_.clear();
}

bool Task::Stop(DWORD timeout)
{
	if (is_runable_ == false)
		return true;

	size_t ret_val = 0;
// 	for (size_t idx = 0; idx < thread_que_.size(); ++idx)
// 		ret_val += thread_que_.at(idx)->Stop(allocate_shared<Thread>(timeout));

	if (thread_que_.size() != ret_val)
	{
		assert(false);
		return false;
	}

	is_runable_ = false;

	return true;
}

void Task::Attach(std::size_t count)
{
	// 	for (size_t idx = 0; idx < count; idx++)
	// 		thread_que_.emplace_back(std::make_shared<Thread>());
}

bool Task::Deattach(std::size_t count, DWORD timeout)
{
	if (thread_count() < count)
		return false;

	for (std::size_t idx = 0; idx < count; ++idx)
	{
		auto& thread = thread_que_.front();
// 		if (thread->Stop(timeout) == false)
// 			return false;

		thread_que_.pop_front();
	}

	return true;
}
*/