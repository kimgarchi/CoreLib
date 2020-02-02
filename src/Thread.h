#pragma once
#include "stdafx.h"
#include "Object.h"

using ThreadID = DWORD;

class Thread : public object
{
public:
    Thread();
    Thread(Func&& func, bool immedidate_run = true);
    
    virtual ~Thread();

    bool Run();
    bool Run(Func&& func);
    bool Stop();

private:
    Func func_;
    std::mutex mtx_;
    std::condition_variable cond_var_;
    std::future<bool> future_;
    std::thread thread_;
    std::atomic_bool is_runable_;
};