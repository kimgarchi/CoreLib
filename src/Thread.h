#pragma once
#include "stdafx.h"
#include "Job.h"

class Thread
{
public:
    Thread(JobNode job);
    Thread(const Thread& thread);
    ~Thread();

    bool Run();
    bool RepeatStop();
    
    inline bool IsRun() { return is_runable_; }

private:
    std::mutex mtx_;
    std::future<bool> future_;
    std::condition_variable cond_var_;    
    std::atomic_bool is_runable_;
    std::thread thread_;
    JobNode job_;
};