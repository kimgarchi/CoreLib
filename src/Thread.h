#pragma once
#include "stdafx.h"
#include "Job.h"

class Thread
{
public:
    Thread(JobBaseNode job, std::atomic_bool& task_runable);
    Thread(const Thread& thread);
    ~Thread();

    bool Stop(DWORD timeout);

private:
    bool is_runable() { return task_runable_.load() && local_runable_.load(); }

    JobBaseNode job_;
    std::future<bool> future_;
    std::thread thread_;
    std::atomic_bool& task_runable_;
    std::atomic_bool local_runable_;
};