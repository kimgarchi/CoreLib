#include "stdafx.h"
#include "Thread.h"

Thread::Thread(JobBaseNode job, CondVar& cond_var, std::atomic_bool& is_runable)
    : job_(job), cond_var_(cond_var), is_runable_(is_runable)
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
    : job_(const_cast<Thread&>(thread).job_), cond_var_(const_cast<Thread&>(thread).cond_var_), is_runable_(const_cast<Thread&>(thread).is_runable_)
{
}

Thread::~Thread()
{
    if (thread_.joinable())
        assert(RepeatStop(0));
}

bool Thread::RepeatStop(DWORD timeout)
{
    if (thread_.joinable() == false)
        return true;
    
    thread_.join();
    return future_.get();    
}