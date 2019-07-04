#pragma once
#include "stdafx.h"
#include "CallBack.h"

class CallBackManager sealed
{
public:
	CallBackManager();
	~CallBackManager();

	bool Initialize();
	bool Stop();

	bool Register(std::shared_ptr<CallBack> callback);

private:

	std::queue<std::shared_ptr<CallBack>> callbacks_;
	std::thread callback_thread_;
	std::mutex mtx_;
	std::atomic<bool> thread_run_;
};