#include "stdafx.h"
#include "Thread.h"

Thread::~Thread()
{
    assert(Stop());
}

bool Thread::Run()
{
    if (is_runable_ == false)
    {
        is_runable_ = true;
        cond_var_.notify_one();
    }

    return true;
}

bool Thread::Stop()
{
    if (is_runable_)
    {
        is_runable_ = false;
        thread_.join();
        return future_.get();
    }

    return true;
}