#pragma once
#include "stdafx.h"

class JobBase;
class Thread
{
public:
    Thread(std::shared_ptr<JobBase> job_ptr, std::atomic_bool& task_runable);
    Thread(const Thread& thread) = delete;
	Thread& operator=(const Thread& thread) = delete;

    ~Thread();

    bool Stop(DWORD timeout);

protected:
    bool is_runable() { return task_runable_.load() && local_runable_.load(); }

private:
	std::shared_ptr<JobBase> job_ptr_;
    std::future<bool> future_;
    std::thread thread_;
    std::atomic_bool& task_runable_;
    std::atomic_bool local_runable_;
};