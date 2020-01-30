#pragma once
#include "stdafx.h"
#include "Object.h"

class Thread : object
{
public:
    using Func = std::function<void()>;

    Thread();
    Thread(Func&& func);
    
    ~Thread();

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