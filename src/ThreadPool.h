#pragma once
#include "stdafx.h"
#include "Thread.h"

class ThreadPool abstract
{
public:
	ThreadPool();
	~ThreadPool();

	bool Init(const std::size_t alloc_thread_count);
	void Clear(DWORD timeout = INFINITE);	
	
protected:
	virtual std::shared_ptr<Thread> makeThread() abstract;

private:
	m_vector<std::shared_ptr<Thread>> threads_;	
	std::shared_ptr<std::mutex> thread_mutex_;	
	std::shared_ptr<std::condition_variable_any> cond_var_;

	std::atomic_bool run_validate_;
};