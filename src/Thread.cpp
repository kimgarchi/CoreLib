#pragma once
#include "stdafx.h"
#include "Thread.h"
#include "Job.h"
#include "LockObject.h"
#include "MemoryAllocator.h"

#include <chrono>

Thread::Thread(const std::atomic_bool& run_validate, std::shared_ptr<std::mutex> thread_mutex, std::shared_ptr<std::condition_variable_any> cond_var, GetJobFunc get_job_func, RemainJobCheckFunc remain_job_check_func)
    : run_validate_(run_validate), job_ptr_(nullptr), job_mutex_(allocate_shared<SyncMutex>()), thread_mutex_(thread_mutex), cond_var_(cond_var), get_job_func_(get_job_func), remain_job_check_func_(remain_job_check_func)
{
    assert(job_mutex_ != nullptr);
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
						return remain_job_check_func_() || run_validate_ == false;
					});
            }

			job_ptr_ = get_job_func_(job_mutex_);
            if (job_ptr_ != nullptr)
            {
#ifdef _DEBUG
                get_job_count_ += 1;
#endif
                job_ptr_->Execute();
            }

        } while (run_validate_ || remain_job_check_func_());

        return true;
    });

    future_ = task.get_future();
    thread_ = std::thread(std::move(task));
}

Thread::~Thread()
{
    assert(Stop());
}

HANDLE Thread::handle()
{
    return thread_.native_handle();
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
        if (job_ptr_ != nullptr)
        {
            /// warn log
            job_ptr_->JobTerminate();
        }
        
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