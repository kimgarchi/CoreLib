#include "stdafx.h"
#include "Thread.h"

Thread::Thread(JobNode job)
    : is_runable_(false), job_(job)
{
    std::packaged_task<bool()> task = std::packaged_task<bool()>(
        [&]()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cond_var_.wait(lock, [&] {return is_runable_ == true; });

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
    : job_(thread.job_)
{
}

Thread::~Thread()
{
    assert(RepeatStop());
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

bool Thread::RepeatStop()
{
    if (is_runable_ == false)
        assert(Run());
    
    is_runable_ = false;
    thread_.join();
    return future_.get();
}