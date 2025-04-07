#include "stdafx.h"
#include "ThreadPool.h"

ThreadPool::ThreadPool()
    : threads_(allocate_vector<std::shared_ptr<Thread>>()), job_queue_(allocate_deque<std::shared_ptr<JobBase>>()),
    thread_mutex_(allocate_shared<std::mutex>()), job_mutex_(allocate_shared<SyncMutex>()), cond_var_(allocate_shared<std::condition_variable_any>()),
    run_validate_(true)
{
    assert(thread_mutex_ != nullptr);
    assert(job_mutex_ != nullptr);
    assert(cond_var_ != nullptr);
}

ThreadPool::~ThreadPool()
{
    Clear();
}

bool ThreadPool::PushJob(std::shared_ptr<JobBase> job_ptr)
{
    if (run_validate_ == false)
        return false;

    SingleLock lock(*job_mutex_);
    job_queue_.push_back(job_ptr);
    
    cond_var_->notify_one();

    return true;
}

std::shared_ptr<JobBase> ThreadPool::GetNextJob(std::shared_ptr<SyncMutex> thread_job_mutex)
{
    SingleLock lock(*job_mutex_);

    if (job_queue_.empty())
    {
        return nullptr;
    }

    std::shared_ptr<JobBase> job_ptr = job_queue_.front();  
    job_queue_.pop_front();

    if (job_ptr == nullptr)
    {
        assert(job_ptr != nullptr);
        return nullptr;
    }
    
    assert(thread_job_mutex != nullptr);

    job_ptr->AttachJobMutex(thread_job_mutex);

    return job_ptr;
}

bool ThreadPool::RemainJobCheck()
{
    SingleLock lock(*job_mutex_);
    
    return job_queue_.empty() ? false : true;
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
        std::shared_ptr<Thread> thread_ptr = allocate_shared<Thread>(run_validate_, thread_mutex_, cond_var_, 
            std::bind(&ThreadPool::GetNextJob, this, std::placeholders::_1), 
            std::bind(&ThreadPool::RemainJobCheck, this));
        if (thread_ptr == nullptr)
        {
            return false;
        }

        threads_.emplace_back(thread_ptr);
    }

    return true;
}

void ThreadPool::Clear(bool is_jobs_terminate, DWORD timeout)
{
    run_validate_ = false;
    if (threads_.empty() == false && is_jobs_terminate == false)
    {
        std::size_t remain_job_count = 0;
		do
		{
            SingleLock lock(*job_mutex_);
            std::this_thread::sleep_for(std::chrono::seconds(1));            
            remain_job_count = job_queue_.size();            
        } while (remain_job_count > 0);
    }
    else
    {
        while (job_queue_.empty() == false)
        {
            SingleLock lock(*job_mutex_);
            job_queue_.pop_front();
        }	
    }

    cond_var_->notify_all();

    for (std::shared_ptr<Thread> thread_ptr : threads_)
    {
#ifdef _DEBUG
        if (thread_ptr->get_job_count() != 0)
            std::cout << thread_ptr->handle() << " : " << thread_ptr->get_job_count() << std::endl;
#endif
        thread_ptr->Stop(timeout);        
    }
}