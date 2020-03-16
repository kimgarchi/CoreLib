#include "stdafx.h"
#include "Thread.h"

Thread::Thread(JobBaseNode job, std::atomic_bool& is_runable)
    : job_(job), is_runable_(is_runable)
{
    std::packaged_task<bool()> task = std::packaged_task<bool()>(
        [&]()
    {
        if (job_->Prepare() == false)
            return false;

        try
        {
            while (is_runable_ && job_->RepeatWork());
        }
        catch (...)
        {
            std::cout << "bad except" << std::endl;
            return false;
        }

        return true;
    });

    future_ = task.get_future();
    thread_ = std::thread(std::move(task));
}

Thread::Thread(const Thread& thread)
    : job_(const_cast<Thread&>(thread).job_), is_runable_(const_cast<Thread&>(thread).is_runable_)
{
}

Thread::~Thread()
{
    if (thread_.joinable())
        assert(RepeatStop(TIME_OUT_INFINITE));
}

bool Thread::RepeatStop(DWORD timeout)
{
    if (thread_.joinable() == false)
        return true;
    
    thread_.join();
    return future_.get();    
}