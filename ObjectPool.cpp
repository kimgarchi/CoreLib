#include "stdafx.h"
#include "ObjectPool.h"

ObjectPool::ObjectPool(DWORD min_object_count, DWORD max_object_count, DWORD variance_object_count)
	: min_idle_object_count_(min_object_count), max_object_count_(max_object_count), variance_object_count_(variance_object_count)
{
}

ObjectPool::~ObjectPool()
{
}

bool ObjectPool::Initialize()
{
	if (IncreaseObjectCount() == false)
	{
		return false;
	}

	return true;
}

OBJECT_STATUS ObjectPool::GetObjectStatus(std::shared_ptr<Object> object)
{
	std::unique_lock<std::mutex> lock(mtx_);
	auto itor = objects_.find(object);
	if (itor != objects_.end())
	{
		assert(false);
		return OBJECT_STATUS::OBJECT_STATUS_UNKNOWN;
	}
	
	return itor->second;
}

std::shared_ptr<Object> ObjectPool::Get()
{
	std::unique_lock<std::mutex> lock(mtx_);
	if (idle_objects_.empty())
	{
		if (IncreaseObjectCount() == false)
			return nullptr;
	}
	
	std::shared_ptr<Object> object = *idle_objects_.begin();
	if (ChangeObjectStatus(object, OBJECT_STATUS::OBJECT_STATUS_USED) == false)
		return nullptr;

	return object;
}

bool ObjectPool::Return(std::shared_ptr<Object> object)
{
	std::unique_lock<std::mutex> lock(mtx_);
	auto itor = used_objects_.find(object);
	if (itor == used_objects_.end())
		return false;

	if (ChangeObjectStatus(object, OBJECT_STATUS::OBJECT_STATUS_INACTIVE_WAIT) == false)
		return false;

	return false;
}

bool ObjectPool::IncreaseObjectCount()
{
	std::unique_lock<std::mutex> lock(mtx_);
	if (objects_.size() >= max_object_count_)
		return false;

	DWORD increase_count = static_cast<DWORD>(max_object_count_ - objects_.size());
	if (increase_count > variance_object_count_)
		increase_count = variance_object_count_;

	for (DWORD i = 0; i < increase_count; ++i)
	{
		std::shared_ptr<Object> object = CreateObject();
		objects_.insert(ChunkStatusMap::value_type(object, OBJECT_STATUS::OBJECT_STATUS_IDLE));
		idle_objects_.insert(object);
	}

	return true;
}

bool ObjectPool::DecreaseObjectCount()
{
	std::unique_lock<std::mutex> lock(mtx_);
	if (idle_objects_.size() <= min_idle_object_count_)
		return false;

	DWORD decrease_count = static_cast<DWORD>(idle_objects_.size() - min_idle_object_count_);
	if (decrease_count > variance_object_count_)
		decrease_count = variance_object_count_;

	for (DWORD i = 0; i < decrease_count; ++i)
	{
		if (idle_objects_.empty())
			break;

		std::shared_ptr<Object> object = *idle_objects_.begin();
		auto itor = objects_.find(object);

		if (itor == objects_.end())
		{
			assert(false);
			break;
		}

		if (object.use_count() != idle_object_use_count_)
		{
			assert(false);
			break;
		}
		
		objects_.erase(object);
		idle_objects_.erase(object);
	}

	return true;
}

bool ObjectPool::ChangeObjectStatus(std::shared_ptr<Object> object, OBJECT_STATUS status)
{
	std::unique_lock<std::mutex> lock(mtx_);
	auto itor = objects_.find(object);
	if (itor == objects_.end())
		return false;

	OBJECT_STATUS& object_status = itor->second;
	switch (object_status)
	{
	case OBJECT_STATUS::OBJECT_STATUS_IDLE:
	{
		if (status == OBJECT_STATUS::OBJECT_STATUS_USED)
		{
			used_objects_.insert(object);
			idle_objects_.erase(object);
		}
		else
		{
			assert(false);
			return false;
		}
	}
	break;
	case OBJECT_STATUS::OBJECT_STATUS_INACTIVE_WAIT:
	{
		if (status == OBJECT_STATUS::OBJECT_STATUS_IDLE)
		{
			idle_objects_.insert(object);
			inactive_wait_objects_.erase(object);
		}
		else
		{
			assert(false);
			return false;
		}
	}
	break;
	case OBJECT_STATUS::OBJECT_STATUS_USED:
	{
		if (status == OBJECT_STATUS::OBJECT_STATUS_INACTIVE_WAIT)
		{
			inactive_wait_objects_.insert(ChunkTickMap::value_type(object, GetTickCount64()));
			used_objects_.erase(object);
		}
		else
		{
			assert(false);
			return false;
		}
	}
	break;
	default:
	{
		assert(false);
		return false;
	}
	break;
	}

	object_status = status;

	return true;
}
