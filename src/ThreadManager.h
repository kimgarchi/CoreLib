#pragma once
#include "stdafx.h"
#include "Thread.h"
#include "Job.h"

class ThreadManager
{
public:
// 	ThreadManager();
// 	~ThreadManager();
// 
// // 	template<typename _Job, typename ..._Tys, is_job<_Job> = nullptr>
// // 	DWORD AttachTask(std::size_t thread_count, _Tys&&... Args);
// 
// 	DWORD AttachTask(std::size_t thread_count, std::shared_ptr<JobBase> job_ptr);
// 	bool DeattachTask(DWORD task_id, DWORD timeout = INFINITE);
// 	bool change_thread_count(DWORD task_id, size_t thread_count);
// 	
// 	bool Stop(DWORD task_id, DWORD timeout = INFINITE);
// 	
// private:
// 	std::size_t thread_count(DWORD task_id);
// 
// 	std::mutex mtx_;
// // 	Tasks tasks_;
// // 	AllocDWORD alloc_task_id_;
};

/*
template<typename _Job, typename ..._Tys, is_job<_Job>>
DWORD ThreadManager::AttachTask(size_t thread_count, _Tys&&... Args)
{
	std::unique_lock<std::mutex> lock(mtx_);

	alloc_task_id_.fetch_add(1);
	std::shared_ptr<JobBase> job = std::make_shared<_Job>(Args...); //allocate_shared<_Job>(Args...);

	if (tasks_.emplace(alloc_task_id_, std::make_shared<Task>(thread_count, job) allocate_shared<Task>(thread_count, job) ).second == false)
		return INVALID_ALLOC_ID;

	return alloc_task_id_;
}
*/