#include "stdafx.h"
#include "Thread.h"
#include "Job.h"

Thread::Thread(std::shared_ptr<JobBase> job_ptr, std::atomic_bool& task_runable)
    : job_ptr_(job_ptr), task_runable_(task_runable), local_runable_(true)
{
    std::packaged_task<bool()> task = std::packaged_task<bool()>(
        [&]()
    {
        try
        {
            while (is_runable() && job_ptr_->Execute());
        }
        catch (...)
        {
            assert(false);            
            return false;
        }
        
        return true;
    });

    future_ = task.get_future();
    thread_ = std::thread(std::move(task));
}

Thread::~Thread()
{
    assert(Stop(INFINITE));
}

bool Thread::Stop(DWORD timeout)
{
    if (is_runable() == false && thread_.joinable())
        return true;

    if (future_.valid() == false)
        return true;

    local_runable_ = false;

    auto ret = future_.wait_for(std::chrono::seconds(timeout));
    switch (ret)
    {
    case std::future_status::deferred:
    case std::future_status::timeout:
    {
        assert(false);
        return false;
    }   
    }

    if (future_.get() == false)
        return false;

    thread_.join();

    return true;
}