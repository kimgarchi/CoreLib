#include "stdafx.h"
#include "CallBackManager.h"

CallBackManager::CallBackManager()
	: thread_run_(false)
{
}

CallBackManager::~CallBackManager()
{
}

bool CallBackManager::Initialize()
{
	thread_run_ = true;
	callback_thread_ = std::thread([&]()
		{
			while (thread_run_)
			{
				std::unique_lock<std::mutex> lock(mtx_);

				ULONGLONG update_tick_count = GetTickCount64();
				size_t queue_size = callbacks_.size();

				for (size_t i = 0; i < queue_size; ++i)
				{
					std::shared_ptr<CallBack> callback = callbacks_.front();
					callbacks_.pop();

					if (callback->IsCall() == false)
					{
						callbacks_.push(callback);
						continue;
					}

					if (callback->ProcessJob() == false)
					{
						//... log...
						assert(false);
					}
				}
			}
		});

	return false;
}

bool CallBackManager::Stop()
{
	thread_run_ = false;
	callback_thread_.join();

	return true;
}

bool CallBackManager::Register(std::shared_ptr<CallBack> callback)
{
	std::unique_lock<std::mutex> lock(mtx_);
	callbacks_.push(callback);

	return true;
}
