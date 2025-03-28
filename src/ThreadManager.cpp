#include "stdafx.h"
#include "ThreadManager.h"
// 
// ThreadManager::ThreadManager()
// 	: alloc_task_id_(0)
// {
// 	//tasks_ = allocate_map<DWORD, std::shared_ptr<Task>>();
// }
// 
// ThreadManager::~ThreadManager()
// {
// 	tasks_.clear();
// }
// 
// LONG ThreadManager::AttachTask(std::size_t thread_count, std::shared_ptr<JobBase> job_ptr)
// {
// 	std::unique_lock<std::mutex> lock(mtx_);
// 
// 	alloc_task_id_.fetch_add(1);
// 	if (tasks_.emplace(alloc_task_id_, std::make_shared<Task>(thread_count, job_ptr)  /*allocate_shared<Task>(thread_count, job_ptr)*/).second == false)
// 		return 0;
// 
// 	return alloc_task_id_;
// }
// 
// bool ThreadManager::DeattachTask(DWORD task_id, DWORD timeout)
// {
// 	std::unique_lock<std::mutex> lock(mtx_);
// 
// 	if (tasks_.find(task_id) == tasks_.end())
// 		return false;
// 
// 	if (tasks_.at(task_id)->is_runable())
// 		assert(tasks_.at(task_id)->Stop(timeout));
// 
// 	return static_cast<bool>(tasks_.erase(task_id));
// }
// 
// bool ThreadManager::change_thread_count(DWORD task_id, size_t thread_count)
// {
// 	std::unique_lock<std::mutex> lock(mtx_);
// 
// 	if (tasks_.find(task_id) == tasks_.end())
// 		return false;
// 
// 	auto& task = tasks_.at(task_id);
// 	size_t prev_count = task->thread_count();
// 	if (prev_count == thread_count)
// 		return true;
// 	else if (prev_count > thread_count)
// 		task->Deattach(prev_count - thread_count);
// 	else
// 		task->Attach(thread_count - prev_count);
// 
// 	if (task->thread_count() == 0)
// 		return tasks_.erase(task_id);
// 
// 	return true;
// }
// 
// bool ThreadManager::Stop(DWORD task_id, DWORD timeout)
// {
// 	std::unique_lock<std::mutex> lock(mtx_);
// 	if (tasks_.find(task_id) == tasks_.end())
// 		return false;
// 
// 	if (tasks_.at(task_id)->Stop(timeout) == false)
// 	{
// 		assert(false);
// 		return false;
// 	}
// 
// 	return static_cast<bool>(tasks_.erase(task_id));
// }
// 
// std::size_t ThreadManager::thread_count(DWORD task_id)
// {
// 	if (tasks_.find(task_id) == tasks_.end())
// 		return 0;
// 
// 	return tasks_.at(task_id)->thread_count();
// }