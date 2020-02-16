#include "stdafx.h"
#include "ThreadManager.h"

ThreadManager::Task::Task(JobHub job, size_t thread_count)
	: job_(job)
{
	threads_.reserve(thread_count);
	for (size_t idx = 0; idx < thread_count; ++idx)
		threads_.emplace_back(job_.make_node());
}

ThreadManager::Task::~Task()
{
	assert(Stop());
}

bool ThreadManager::Task::Run()
{
	size_t ret_val = 0;
	for (size_t idx = 0; idx < threads_.size(); ++idx)
		ret_val += threads_.at(idx).Run();
	
	if (threads_.size() != ret_val)
	{
		Stop();
		return false;
	}

	return true;
}

bool ThreadManager::Task::Stop()
{
	size_t ret_val = 0;
	for (size_t idx = 0; idx < threads_.size(); ++idx)
		ret_val += threads_.at(idx).RepeatStop();

	if (threads_.size() != ret_val)
	{
		assert(false);
		return false;
	}
	
	return true;
}

bool ThreadManager::DeattachTask(TaskID task_id)
{
	std::unique_lock<std::mutex> lock(mtx_);
	
	if (tasks_.find(task_id) == tasks_.end())
		return false;

	tasks_.at(task_id)->Stop();
	return static_cast<bool>(tasks_.erase(task_id));
}

bool ThreadManager::Run(TaskID task_id)
{
	std::unique_lock<std::mutex> lock(mtx_);
	if (tasks_.find(task_id) == tasks_.end())
		return false;

	return tasks_.at(task_id)->Run();
}

bool ThreadManager::Stop(TaskID task_id)
{
	std::unique_lock<std::mutex> lock(mtx_);
	if (tasks_.find(task_id) == tasks_.end())
		return false;

	return tasks_.at(task_id)->Stop();
}