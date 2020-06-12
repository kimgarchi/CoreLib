#pragma once
#include "stdafx.h"
#include "Thread.h"
#include "singleton.h"

class ThreadManager : public Singleton<ThreadManager>
{
private:
	class Task : public object
	{
	private:
		using ThreadQue = std::deque<Thread>;

	public:
		Task(JobBaseHub job, size_t thread_count);
		virtual ~Task();

		bool Stop(DWORD timeout = INFINITE);
		inline bool is_runable() const { return is_runable_.load(); }
		void Attach(size_t count);
		bool Deattach(size_t count, DWORD timeout = INFINITE);

		size_t thread_count() { return thread_que_.size(); }

	private:		
		JobBaseHub job_;
		ThreadQue thread_que_;
		std::atomic_bool is_runable_;
	};

	DEFINE_WRAPPER_HUB(Task);
	using AllocTaskID = std::atomic<TaskID>;
	using Tasks = std::map<TaskID, TaskHub>;

public:
	ThreadManager()
		: alloc_task_id_(0)
	{}

	~ThreadManager() { tasks_.clear(); }

	template<typename _Job, typename ..._Tys, is_job<_Job> = nullptr>
	TaskID AttachTask(size_t thread_count, _Tys&&... Args);
	TaskID AttachTask(size_t thread_count, JobBaseHub job);
	bool DeattachTask(TaskID task_id, DWORD timeout = INFINITE);
	bool change_thread_count(TaskID task_id, size_t thread_count);
	
	bool Stop(TaskID task_id, DWORD timeout = INFINITE);
	
private:
	size_t thread_count(TaskID task_id);

	std::mutex mtx_;
	Tasks tasks_;
	AllocTaskID alloc_task_id_;
};

template<typename _Job, typename ..._Tys, is_job<_Job>>
TaskID ThreadManager::AttachTask(size_t thread_count, _Tys&&... Args)
{
	std::unique_lock<std::mutex> lock(mtx_);

	alloc_task_id_.fetch_add(1);
	JobBaseHub job = make_wrapper_hub<_Job>(Args...);

	if (tasks_.emplace(alloc_task_id_, make_wrapper_hub<Task>(job, thread_count)).second == false)
		return INVALID_ALLOC_ID;

	return alloc_task_id_;
}