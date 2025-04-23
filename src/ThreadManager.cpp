#include "stdafx.h"
#include "ThreadManager.h"

#include "ThreadPool.h"

ThreadManager::ThreadManager()
	: thread_pools_(allocate_map<std::size_t, std::shared_ptr<ThreadPool>>())
{
}

bool ThreadManager::AttachPool(const std::size_t key, const std::size_t count)
{
	auto thread_pool = allocate_shared<ThreadPool>();
	if (thread_pool == nullptr)
	{
		assert(false);
		return false;
	}

	if (thread_pool->Init(count) == false)
	{
		assert(false);
		return false;
	}

	return thread_pools_.emplace(std::pair(key, thread_pool)).second;
}
