#pragma once
#include "singleton.h"
#include "Thread.h"
#include "Wrapper.h"

using TaskID = size_t;
using ThreadID = size_t;
class ThreadManager : public Singleton<ThreadManager>
{
private:
	class Task;
	using ThreadHub = wrapper_hub<Thread>;
	using ThreadNode = wrapper_node<Thread>;
	using Threads = std::vector<ThreadHub>;
	using AllocTaskID = std::atomic<TaskID>;
	using Tasks = std::map<TaskID, Task>;

	class Task
	{
	public:
		template<typename _Func, typename ..._Tys>
		Task(const _Func& func, const size_t& thread_count, _Tys&&... Args);
		~Task();

		bool Run();
		bool Stop();

		size_t thread_count() { return threads_.size(); }

	private:
		Threads threads_;
	};

public:
	ThreadManager()
		: alloc_task_id_(0)
	{}

	template<typename _Func, typename ..._Tys>
	TaskID AttachTask(const _Func& func, const size_t& thread_count, _Tys&&... Args);
	bool DeattachTask(TaskID task_id);

	bool Run(TaskID task_id);
	bool Stop(TaskID task_id);

private:
	std::mutex mtx_;
	Tasks tasks_;
	AllocTaskID alloc_task_id_;
};

template<typename _Func, typename ..._Tys>
ThreadManager::Task::Task(const _Func& func, const size_t& thread_count, _Tys&&... Args)
{	
	threads_.reserve(thread_count);
	for (size_t idx = 0; idx < thread_count; ++idx)
		threads_.emplace_back(make_wrapper_hub<Thread>(func, Args...));
}

template<typename _Func, typename ..._Tys>
TaskID ThreadManager::AttachTask(const _Func& func, const size_t& thread_count, _Tys&& ...Args)
{
	std::unique_lock<std::mutex> lock(mtx_);
	
	auto task_id = alloc_task_id_.fetch_add(1);
	if (tasks_.emplace(task_id, Task(func, thread_count, Args...)).second == false)
		return INVALID_ALLOC_ID;
	
	return task_id;
}