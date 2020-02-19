#pragma once
#include "stdafx.h"
#include "Thread.h"
#include "singleton.h"

#ifdef _DEBUG
const static DWORD _default_thread_stop_timeout_ = 1000;
#else
const static DWORD _default_thread_stop_timeout_ = 0;
#endif

using TaskID = size_t;
using ThreadID = size_t;
class ThreadManager : public Singleton<ThreadManager>
{
private:
	using Threads = std::vector<Thread>;

	class Task : public object
	{
	public:
		Task(JobBaseHub job, size_t thread_count);
		~Task();

		void Run();
		bool Stop(DWORD timeout);
		inline bool is_runable() { return is_runable_.load(); }

		size_t thread_count() { return threads_.size(); }

	private:
		JobBaseHub job_;
		Threads threads_;
		CondVar cond_var_;
		std::atomic_bool is_runable_;
	};

	DEFINE_WRAPPER_HUB(Task);
	using AllocTaskID = std::atomic<TaskID>;
	using Tasks = std::map<TaskID, TaskHub>;

public:
	ThreadManager()
		: alloc_task_id_(0)
	{}

	template<typename _Job, typename ..._Tys, is_job<_Job> = nullptr>
	TaskID AttachTask(size_t thread_count, _Tys&&... Args);
	bool DeattachTask(TaskID task_id, DWORD timeout = _default_thread_stop_timeout_);

	bool Run(TaskID task_id);
	bool Stop(TaskID task_id, DWORD timeout = _default_thread_stop_timeout_);

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
	JobBaseHub job = make_wrapper_hub<_Job>(Args...);

	if (tasks_.emplace(task_id, make_wrapper_hub<Task>(job, thread_count)).second == false)
		return INVALID_ALLOC_ID;

	return task_id;
}
