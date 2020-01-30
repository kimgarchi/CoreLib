#include "stdafx.h"
#include "Thread.h"

Thread::Thread()
    : func_(nullptr), is_runable_(false)
{
    std::packaged_task<bool()> task = std::packaged_task<bool()>(
        [&]()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cond_var_.wait(lock, [&] {return (is_runable_ == true && func_ != nullptr); });
        
        while (is_runable_)
        {
            try
            {
                func_();
            }
            catch (...)
            {
                std::cout << "bad except" << std::endl;
                return false;
            }
        }

        return true;
    });

    future_ = task.get_future();
    thread_ = std::thread(std::move(task));
}

Thread::Thread(Func&& func)
    : func_(func), is_runable_(false)
{
    std::packaged_task<bool()> task = std::packaged_task<bool()>(
        [&]()
    {
        {
            std::unique_lock<std::mutex> lock(mtx_);
            cond_var_.wait(lock, [&] {return is_runable_ == true; });
        }

        while (is_runable_)
        {
            try
            {
                func_();


            }
            catch (...)
            {
                std::cout << "bad except" << std::endl;
                return false;
            }
        }

        return true;
    });

    future_ = task.get_future();
    thread_ = std::thread(std::move(task));
}

Thread::~Thread()
{
    assert(Stop());
}

bool Thread::Run()
{
    if (func_ == nullptr)
    {
        assert(false);
        return false;
    }

    if (is_runable_ == false)
    {
        is_runable_ = true;
        cond_var_.notify_one();
    }

    return true;
}

bool Thread::Run(Func&& func)
{
    if (func == nullptr)
    {
        assert(false);
        return false;
    }

    func_ = std::forward<Func>(func);
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