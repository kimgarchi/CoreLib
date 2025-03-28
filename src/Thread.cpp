#include "stdafx.h"
#include "Thread.h"
#include "Job.h"
#include "LockObject.h"
#include "MemoryAllocator.h"

Thread::Thread(std::shared_ptr<SyncEvent> sync_event)
    : job_ptr_(nullptr), run_validate_(true), sync_event_(sync_event), sync_mutex_(allocate_shared<SyncMutex>())
{
    assert(sync_event_ != nullptr);
    assert(sync_mutex_ != nullptr);

    std::packaged_task<bool()> task = std::packaged_task<bool()>(
        [&]()
    {
        try
        {
            do 
            {
                const DWORD ret = sync_event_->wait_signaled();
                if (ret == WAIT_OBJECT_0)
                {
                    SingleLock lock(*sync_mutex_);
					if (job_ptr_ == nullptr)
					{
						assert(false);
						continue;
					}

                    job_ptr_->Execute();
                    job_ptr_ = nullptr;
                }

            } while (run_validate_);
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

bool Thread::AttachJob(std::shared_ptr<JobBase> job_ptr)
{
    SingleLock lock(*sync_mutex_);

    if (job_ptr_ != nullptr)
    {
        assert(false);
        return false;
    }

    job_ptr_ = job_ptr;

    sync_event_->raise_signaled();

    return true;
}

bool Thread::Stop(DWORD timeout)
{
    SingleLock lock(*sync_mutex_);

    if (thread_.joinable())
        return true;

    if (future_.valid() == false)
        return true;

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