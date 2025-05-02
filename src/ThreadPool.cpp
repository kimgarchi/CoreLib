#include "stdafx.h"
#include "ThreadPool.h"

ThreadPool::ThreadPool()
    : threads_(allocate_vector<std::shared_ptr<Thread>>()),
    thread_mutex_(allocate_shared<std::mutex>()), cond_var_(allocate_shared<std::condition_variable_any>()),
    run_validate_(true)
{
    assert(thread_mutex_ != nullptr);
    assert(cond_var_ != nullptr);
}

ThreadPool::~ThreadPool()
{
    Clear();
}

bool ThreadPool::Init(const std::size_t alloc_thread_count)
{
    if (threads_.empty() == false)
    {
        return false;
    }

    threads_.reserve(alloc_thread_count);

    for(std::size_t count = 0; count < alloc_thread_count; ++count)
    {
        std::shared_ptr<Thread> thread_ptr = makeThread();/* allocate_shared<Thread>(run_validate_, thread_mutex_, cond_var_);*/
        if (thread_ptr == nullptr)
        {
            return false;
        }

        threads_.emplace_back(thread_ptr);
    }

    return true;
}

void ThreadPool::Clear(DWORD timeout)
{
    run_validate_ = false;    
    cond_var_->notify_all();

    for (auto thread_ptr : threads_)
    {
        thread_ptr->Stop(timeout);
    }
}