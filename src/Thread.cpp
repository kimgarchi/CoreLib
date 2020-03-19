#include "stdafx.h"
#include "Thread.h"

Thread::Thread(JobBaseNode job, std::atomic_bool& task_runable)
    : job_(job), task_runable_(task_runable), local_runable_(true)
{
    std::packaged_task<bool()> task = std::packaged_task<bool()>(
        [&]()
    {
        try
        {
            while (is_runable() && job_->RepeatWork());
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
    : job_(const_cast<Thread&>(thread).job_), 
        task_runable_(const_cast<Thread&>(thread).task_runable_), local_runable_(true)
{
}

Thread::~Thread()
{
    assert(Stop(TIME_OUT_INFINITE));
}

bool Thread::Stop(DWORD timeout)
{
    if (is_runable() == false)
        return false;

    if (thread_.joinable() == false)
        return false;

    switch (future_.wait_for(std::chrono::seconds(timeout)))
    {
    case std::future_status::deferred:
    case std::future_status::timeout:
        return false;
    }

    return future_.get();
}