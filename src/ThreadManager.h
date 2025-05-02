#pragma once
#include "stdafx.h"

class ThreadPool;
class ThreadManager
{
public:
	ThreadManager();

	bool AttachPool(const std::size_t key, const std::size_t count);
	

private:
	//m_map<std::size_t, std::shared_ptr<ThreadPool>> thread_pools_;
};