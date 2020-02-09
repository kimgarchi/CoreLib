#pragma once
#include "stdafx.h"
#include "Object.h"

class Thread : public object
{
public:
    template<typename _Func, typename ..._Tys>
    Thread(const _Func& func, _Tys&&... Args);
    virtual ~Thread();

    bool Run();
    bool Stop();

    bool IsRun() { return is_runable_; }

private:
    std::mutex mtx_;
    std::future<bool> future_;
    std::condition_variable cond_var_;    
    std::atomic_bool is_runable_;
    std::thread thread_;    
};

template<typename _Func, typename ..._Tys>
Thread::Thread(const _Func& func, _Tys&&... Args)
    : is_runable_(false)
{
    std::packaged_task<bool()> task = std::packaged_task<bool()>(
        [&]()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cond_var_.wait(lock, [&] {return is_runable_ == true; });

        try
        {
            func(Args...);
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