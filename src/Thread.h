#pragma once
#include "stdafx.h"

class Thread abstract : public NonCopyableBase
{
public:    
	Thread(const std::atomic_bool& run_validate, std::shared_ptr<std::mutex> thread_mutex, std::shared_ptr<std::condition_variable_any> cond_var);
    virtual ~Thread();

	virtual bool DoWork() abstract;
    bool Stop(DWORD timeout = INFINITE);

private:
	const std::atomic_bool& run_validate_;
    std::future<bool> future_;
    std::thread thread_;
    std::shared_ptr<std::mutex> thread_mutex_;
    std::shared_ptr<std::condition_variable_any> cond_var_;    
};