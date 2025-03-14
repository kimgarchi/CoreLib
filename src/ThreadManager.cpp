#include "stdafx.h"
#include "ThreadManager.h"

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
	for (size_t idx = 0; idx < thread_que_.size(); ++idx)
		ret_val += thread_que_.at(idx)->Stop(timeout);

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
	for (size_t idx = 0; idx < count; idx++)
		thread_que_.emplace_back(std::make_shared<Thread>(job_ptr_, std::ref(is_runable_)));
}

bool Task::Deattach(std::size_t count, DWORD timeout)
{
	if (thread_count() < count)
		return false;

	for (std::size_t idx = 0; idx < count; ++idx)
	{
		auto& thread = thread_que_.front();
		if (thread->Stop(timeout) == false)
			return false;

		thread_que_.pop_front();
	}

	return true;
}

ThreadManager::ThreadManager()
	: alloc_task_id_(0)
{
	//tasks_ = allocate_map<TaskID, std::shared_ptr<Task>>();
}

ThreadManager::~ThreadManager()
{
	tasks_.clear();
}

TaskID ThreadManager::AttachTask(std::size_t thread_count, std::shared_ptr<JobBase> job_ptr)
{
	std::unique_lock<std::mutex> lock(mtx_);

	alloc_task_id_.fetch_add(1);
	if (tasks_.emplace(alloc_task_id_, std::make_shared<Task>(thread_count, job_ptr)  /*allocate_shared<Task>(thread_count, job_ptr)*/).second == false)
		return INVALID_ALLOC_ID;

	return alloc_task_id_;
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

bool ThreadManager::change_thread_count(TaskID task_id, size_t thread_count)
{
	std::unique_lock<std::mutex> lock(mtx_);

	if (tasks_.find(task_id) == tasks_.end())
		return false;

	auto& task = tasks_.at(task_id);
	size_t prev_count = task->thread_count();
	if (prev_count == thread_count)
		return true;
	else if (prev_count > thread_count)
		task->Deattach(prev_count - thread_count);
	else
		task->Attach(thread_count - prev_count);

	if (task->thread_count() == 0)
		return tasks_.erase(task_id);

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

std::size_t ThreadManager::thread_count(TaskID task_id)
{
	if (tasks_.find(task_id) == tasks_.end())
		return 0;

	return tasks_.at(task_id)->thread_count();
}