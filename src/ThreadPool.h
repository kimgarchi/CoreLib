#pragma once
#include "stdafx.h"
#include "Thread.h"
#include "Job.h"

class ThreadPool
{
public:
	ThreadPool();
	~ThreadPool();

	bool Init(const std::size_t alloc_thread_count);
	void Clear(bool is_jobs_terminate = false, DWORD timeout = INFINITE);
	bool PushJob(std::shared_ptr<JobBase> job_ptr);
	std::shared_ptr<JobBase> GetNextJob(std::shared_ptr<SyncMutex> thread_job_mutex);	
	bool RemainJobCheck();
	
private:
	using ThreadAllocator = MemoryAllocator<std::shared_ptr<Thread>>;
	using Threads = std::vector<std::shared_ptr<Thread>, ThreadAllocator>;

	using JobAllocator = MemoryAllocator<std::shared_ptr<JobBase>>;
	using JobQueue = std::deque<std::shared_ptr<JobBase>, JobAllocator>;

	Threads threads_;	
	JobQueue job_queue_;
	std::shared_ptr<std::mutex> thread_mutex_;
	std::shared_ptr<SyncMutex> job_mutex_;
	std::shared_ptr<std::condition_variable_any> cond_var_;

	std::atomic_bool run_validate_;
};