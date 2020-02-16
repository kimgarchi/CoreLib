#pragma once
#include "singleton.h"
#include "Thread.h"

using TaskID = size_t;
using ThreadID = size_t;
class ThreadManager : public Singleton<ThreadManager>
{
private:
	class Task;
	using Threads = std::vector<Thread>;
	using TaskHub = wrapper_hub<Task>;
	using AllocTaskID = std::atomic<TaskID>;
	using Tasks = std::map<TaskID, TaskHub>;
	
	class Task : public object
	{
	public:
		Task(JobHub job, size_t thread_count);
		~Task();

		bool Run();
		bool Stop();

		size_t thread_count() { return threads_.size(); }

	private:
		JobHub job_;
		Threads threads_;
	};

public:
	ThreadManager()
		: alloc_task_id_(0)
	{}

	template<typename _Job, typename ..._Tys, is_job<_Job> = nullptr>
	TaskID AttachTask(size_t thread_count, _Tys&&... Args);
	bool DeattachTask(TaskID task_id);

	bool Run(TaskID task_id);
	bool Stop(TaskID task_id);

private:
	std::mutex mtx_;
	Tasks tasks_;
	AllocTaskID alloc_task_id_;
};

template<typename _Job, typename ..._Tys, is_job<_Job>>
TaskID ThreadManager::AttachTask(size_t thread_count, _Tys&&... Args)
{
	std::unique_lock<std::mutex> lock(mtx_);

	auto task_id = alloc_task_id_.fetch_add(1);
	JobHub job = make_wrapper_hub<_Job>(Args...);

	if (tasks_.emplace(task_id, make_wrapper_hub<Task>(job, thread_count)).second == false)
		return INVALID_ALLOC_ID;

	return task_id;
}
