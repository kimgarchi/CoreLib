#include "stdafx.h"
#include "ThreadManager.h"

ThreadManager::Task::Task(JobBaseHub job, size_t thread_count)
	: job_(job), is_runable_(false)
{
	threads_.reserve(thread_count);
	for (size_t idx = 0; idx < thread_count; ++idx)
		threads_.emplace_back(job_.make_node(), std::ref(cond_var_), std::ref(is_runable_));
}

ThreadManager::Task::~Task()
{
	assert(Stop(_default_thread_stop_timeout_));

	threads_.clear();
}

void ThreadManager::Task::Run()
{
	cond_var_.notify_all();
	is_runable_ = true;
}

bool ThreadManager::Task::Stop(DWORD timeout)
{
	if (is_runable_ == false)
		return true;

	is_runable_ = false;
	size_t ret_val = 0;
	for (size_t idx = 0; idx < threads_.size(); ++idx)
		ret_val += threads_.at(idx).RepeatStop(timeout);

	if (threads_.size() != ret_val)
	{
		assert(false);
		return false;
	}
	
	return true;
}

bool ThreadManager::DeattachTask(TaskID task_id, DWORD timeout)
{
	std::unique_lock<std::mutex> lock(mtx_);
	
	if (tasks_.find(task_id) == tasks_.end())
		return false;

	if (tasks_.at(task_id)->is_runable())
		assert(tasks_.at(task_id)->Stop(timeout));

	return static_cast<bool>(tasks_.erase(task_id));
}

bool ThreadManager::Run(TaskID task_id)
{
	std::unique_lock<std::mutex> lock(mtx_);
	if (tasks_.find(task_id) == tasks_.end())
		return false;

	tasks_.at(task_id)->Run();
	return true;
}

bool ThreadManager::Stop(TaskID task_id, DWORD timeout)
{
	std::unique_lock<std::mutex> lock(mtx_);
	if (tasks_.find(task_id) == tasks_.end())
		return false;

	if (tasks_.at(task_id)->Stop(timeout) == false)
	{
		assert(false);
		return false;
	}

	return static_cast<bool>(tasks_.erase(task_id));
}