#pragma once
#include "stdafx.h"

enum THREAD_STATE
{
	INIT = 0,
	WAIT_COND_VAR = 1,
	WAIT_JOB = 2,
	JOB_PROCESS = 3,
};

class SyncMutex;
class SyncEvent;
class JobBase;
class Thread
{
public:    
	using GetJobFunc = std::function<std::shared_ptr<JobBase>(std::shared_ptr<SyncMutex>)>;
    using RemainJobCheckFunc = std::function<bool()>;

    Thread(const std::atomic_bool& run_validate, std::shared_ptr<std::mutex> thread_mutex, std::shared_ptr<std::condition_variable_any> cond_var, GetJobFunc get_job_func, RemainJobCheckFunc job_finish_notify_func);
    Thread(const Thread& thread) = delete;
	Thread& operator=(const Thread& thread) = delete;

    ~Thread();

    HANDLE handle();
    bool Stop(DWORD timeout = INFINITE);

#ifdef  _DEBUG
    std::size_t get_job_count() const { return get_job_count_; }
#endif

private:
	const std::atomic_bool& run_validate_;
    std::future<bool> future_;
    std::thread thread_;
    
    std::shared_ptr<JobBase> job_ptr_;

    std::shared_ptr<SyncMutex> job_mutex_;
    std::shared_ptr<std::mutex> thread_mutex_;
    std::shared_ptr<std::condition_variable_any> cond_var_;
    GetJobFunc get_job_func_;
    RemainJobCheckFunc remain_job_check_func_;

    THREAD_STATE state_{ INIT };

#ifdef  _DEBUG
    std::size_t get_job_count_{ 0 };
#endif

};