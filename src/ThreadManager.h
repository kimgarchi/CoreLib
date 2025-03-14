#pragma once
#include "stdafx.h"
#include "Thread.h"
#include "Job.h"

using TaskID = std::size_t;
using TaskIDs = std::vector<TaskID>;

class Task
{
public:
	Task(std::size_t thread_count, std::shared_ptr<JobBase> job_ptr);
	~Task();

	bool Stop(DWORD timeout = INFINITE);
	inline bool is_runable() const { return is_runable_.load(); }
	void Attach(std::size_t count);
	bool Deattach(std::size_t count, DWORD timeout = INFINITE);

	size_t thread_count() { return thread_que_.size(); }

private:
	const std::shared_ptr<JobBase> job_ptr_;
	std::deque<std::shared_ptr<Thread>> thread_que_;
	std::atomic_bool is_runable_;
};

using AllocTaskID = std::atomic<TaskID>;
using Tasks = std::map<TaskID, std::shared_ptr<Task>>;

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

	template<typename _Job, typename ..._Tys, is_job<_Job> = nullptr>
	TaskID AttachTask(std::size_t thread_count, _Tys&&... Args);

	TaskID AttachTask(std::size_t thread_count, std::shared_ptr<JobBase> job_ptr);
	bool DeattachTask(TaskID task_id, DWORD timeout = INFINITE);
	bool change_thread_count(TaskID task_id, size_t thread_count);
	
	bool Stop(TaskID task_id, DWORD timeout = INFINITE);
	
private:
	std::size_t thread_count(TaskID task_id);

	std::mutex mtx_;
	Tasks tasks_;
	AllocTaskID alloc_task_id_;
};

template<typename _Job, typename ..._Tys, is_job<_Job>>
TaskID ThreadManager::AttachTask(size_t thread_count, _Tys&&... Args)
{
	std::unique_lock<std::mutex> lock(mtx_);

	alloc_task_id_.fetch_add(1);
	std::shared_ptr<JobBase> job = std::make_shared<_Job>(Args...); //allocate_shared<_Job>(Args...);

	if (tasks_.emplace(alloc_task_id_, std::make_shared<Task>(thread_count, job) /*allocate_shared<Task>(thread_count, job)*/ ).second == false)
		return INVALID_ALLOC_ID;

	return alloc_task_id_;
}