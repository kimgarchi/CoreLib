#pragma once
#include "stdafx.h"

class SyncMutex;
class SyncEvent;
class JobBase;
class Thread
{
public:
    Thread(std::shared_ptr<SyncEvent> sync_event);

    Thread(const Thread& thread) = delete;
	Thread& operator=(const Thread& thread) = delete;

    ~Thread();

    bool AttachJob(std::shared_ptr<JobBase> job_ptr);

    bool Stop(DWORD timeout);    

private:
	std::shared_ptr<JobBase> job_ptr_;
    std::shared_ptr<SyncEvent> sync_event_;
    std::shared_ptr<SyncMutex> sync_mutex_;
    std::atomic_bool run_validate_;    
    std::future<bool> future_;
    std::thread thread_;
};