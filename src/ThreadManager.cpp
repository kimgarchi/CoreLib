#include "stdafx.h"
#include "ThreadManager.h"

#include "ThreadPool.h"

ThreadManager::ThreadManager()
	//: thread_pools_(allocate_map<std::size_t, std::shared_ptr<ThreadPool>>())
{
}

bool ThreadManager::AttachPool(const std::size_t key, const std::size_t count)
{
	return true;
}
