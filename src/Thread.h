#pragma once
#include "stdafx.h"
#include "Job.h"

class Thread
{
public:
    Thread(JobBaseNode job, CondVar& cond_var, std::atomic_bool& is_runable);
    Thread(const Thread& thread);
    ~Thread();

    bool RepeatStop(DWORD timeout);

private:
    std::mutex mtx_;
    std::future<bool> future_;
    std::thread thread_;
    JobBaseNode job_;

    CondVar& cond_var_;
    std::atomic_bool& is_runable_;
};