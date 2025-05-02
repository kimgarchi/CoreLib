#pragma once
#include "stdafx.h"
#include "Thread.h"
#include "LockObject.h"

Thread::Thread(const std::atomic_bool& run_validate, std::shared_ptr<std::mutex> thread_mutex, std::shared_ptr<std::condition_variable_any> cond_var)
    : run_validate_(run_validate), thread_mutex_(thread_mutex), cond_var_(cond_var)
{
    assert(thread_mutex != nullptr);

    std::packaged_task<bool()> task = std::packaged_task<bool()>(
        [&]()
    {
        do
        {
            {
                std::unique_lock lock(*thread_mutex_);
				cond_var_->wait(lock,
					[&]()
					{
						return run_validate_ == false;
					});
            }

            if (run_validate_ != false)
                assert(DoWork());

        } while (run_validate_);

        return true;
    });

    future_ = task.get_future();
    thread_ = std::thread(std::move(task));
}

Thread::~Thread()
{
    assert(Stop());
}

bool Thread::Stop(DWORD timeout)
{
    if (thread_.joinable() == false)
        return true;

    if (run_validate_ != false)
        return false;

    auto ret_status = future_.wait_for(std::chrono::seconds(timeout));
    switch (ret_status)
    {
    case std::future_status::deferred:
    case std::future_status::timeout:
    {
        assert(false);
        return false;
    }
    case std::future_status::ready:
        /// ... 
        break;
    }

    if (future_.get() == false)
        return false;

    thread_.join();

    return true;
}