#pragma once
#include "stdafx.h"
#include "Object.h"

enum class OBJECT_STATUS
{
	OBJECT_STATUS_UNUSED,
	OBJECT_STATUS_USED,
};

template<typename T>
class ObjectPool : final
{
public:
	static bool Initialize(DWORD min_object_count, DWORD max_object_count, DWORD variance_object_count);
private:
	ObjectPool<T>();
	~ObjectPool<T>();
	
	ObjectPool<T>(const ObjectPool<T>&) = delete;
	void operator=(const ObjectPool<T>&) = delete;

	bool Initialize();

	OBJECT_STATUS GetObjectStatus(std::shared_ptr<T> object);
	T* Push();
	bool Pop(T*);

	bool Return(std::shared_ptr<T> object);

	DWORD GetIdleChunkSize() { return static_cast<DWORD>(idle_objects_.size()); }
	DWORD GetUseChunkSize() { return static_cast<DWORD>(used_objects_.size()); }
	DWORD GetTotalChunkSize() { return static_cast<DWORD>(GetIdleChunkSize() + GetUseChunkSize()); }
	
	using ChunkSet = std::set<T*>;
	using ChunkSetByStatus = std::map<OBJECT_STATUS, ChunkSet>;
	using ChunkMap = std::map<T*, OBJECT_STATUS>;
	
	bool IncreasePool();
	bool DecreasePool();
	
	bool ChangeObjectStatus(std::shared_ptr<T> object, OBJECT_STATUS status);
	
	ChunkSetByStatus chunk_by_status_;
	ChunkMap chunks_;

	static DWORD min_idle_object_count_;
	static DWORD max_object_count_;
	static DWORD variance_object_count_;

	std::mutex io_mtx_;
};

template<typename T>
ObjectPool<T>::ObjectPool()
	: min_idle_object_count_(0), max_object_count_(0), variance_object_count_(0)
{
}

template<typename T>
ObjectPool<T>::~ObjectPool()
{
}

template<typename T>
bool ObjectPool<T>::Initialize(DWORD min_object_count, DWORD max_object_count, DWORD variance_object_count)
{
	if (min_object_count > max_object_count)
		return false;

	if (max_object_count - min_object_count <= variance_object_count)
		return false;

	ObjectPool<T>::min_idle_object_count_ = min_object_count;
	ObjectPool<T>::max_object_count_ = max_object_count;
	ObjectPool<T>::variance_object_count_ = variance_object_count;

	return true;
}

template<typename T>
bool ObjectPool<T>::Initialize()
{
	if (IncreasePool() == false)
		return false;
	
	return true;
}

template<typename T>
OBJECT_STATUS ObjectPool<T>::GetObjectStatus(std::shared_ptr<T> object)
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

template<typename T>
T* ObjectPool<T>::Push()
{
	
	{
		std::unique_lock<std::mutex> lock(io_mtx_);

		if (idle_objects_.empty())
		{
			if (IncreasePool() == false)
				return nullptr;
		}

		object = *idle_objects_.begin();
		if (ChangeObjectStatus(object, OBJECT_STATUS::OBJECT_STATUS_USED) == false)
			return nullptr;
	}

	return object;
}

template<typename T>
inline bool ObjectPool<T>::Pop(T*)
{


	return false;
}

template<typename T>
bool ObjectPool<T>::Return(std::shared_ptr<T> object)
{
	{
		std::unique_lock<std::mutex> lock(mtx_);
		auto itor = used_objects_.find(object);
		if (itor == used_objects_.end())
			return false;
	}
	
	if (ChangeObjectStatus(object, OBJECT_STATUS::OBJECT_STATUS_INACTIVE_WAIT) == false)
		return false;

	return false;
}

template<typename T>
bool ObjectPool<T>::IncreasePool()
{
	std::unique_lock<std::mutex> lock(io_mtx_);
	{
		if (objects_.size() >= max_object_count_)
			return false;

		DWORD increase_count = static_cast<DWORD>(max_object_count_ - objects_.size());
		if (increase_count > variance_object_count_)
			increase_count = variance_object_count_;

		for (DWORD i = 0; i < increase_count; ++i)
		{
			std::shared_ptr<T> object = CreateObject();
			objects_.insert(ChunkStatusMap::value_type(object, OBJECT_STATUS::OBJECT_STATUS_IDLE));
			idle_objects_.insert(object);
		}
	}

	return true;
}

template<typename T>
inline bool ObjectPool<T>::DecreasePool()
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

		std::shared_ptr<T> object = *idle_objects_.begin();
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

template<typename T>
bool ObjectPool<T>::ChangeObjectStatus(std::shared_ptr<T> object, OBJECT_STATUS status)
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