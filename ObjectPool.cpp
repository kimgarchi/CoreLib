#include "stdafx.h"
#include "ObjectPool.h"

template<typename _Ty>
ObjectPool<_Ty>::ObjectPool()
	: min_idle_object_count_(0), max_object_count_(0), variance_object_count_(0)
{
}

template<typename _Ty>
ObjectPool<_Ty>::~ObjectPool()
{
	while (chunks_.empty() == false)
		DecreasePool();
}

template<typename _Ty>
bool ObjectPool<_Ty>::Initialize(DWORD min_object_count, DWORD max_object_count, DWORD variance_object_count)
{
	if (min_object_count > max_object_count)
		return false;

	if (max_object_count - min_object_count <= variance_object_count)
		return false;

	ObjectPool<_Ty>::min_idle_object_count_ = min_object_count;
	ObjectPool<_Ty>::max_object_count_ = max_object_count;
	ObjectPool<_Ty>::variance_object_count_ = variance_object_count;

	return true;
}

template<typename _Ty>
bool ObjectPool<_Ty>::Initialize()
{
	if (IncreasePool() == false)
		return false;

	return true;
}

template<typename _Ty>
bool ObjectPool<_Ty>::Push(_Ty* object)
{
	return true;
}

template<typename _Ty>
_Ty* ObjectPool<_Ty>::Pop()
{
	_Ty* object = nullptr;
	{
		std::unique_lock<std::mutex> lock(mtx_);

		if (chunk_que_.empty())
		{
			if (IncreasePool() == false)
				return nullptr;
		}

		object = chunk_que_.begin();
	}

	return object;
}

template<typename _Ty>
bool ObjectPool<_Ty>::IncreasePool()
{
	std::unique_lock<std::mutex> lock(mtx_);
	{
		if (chunks_.size() >= max_object_count_)
			return false;

		DWORD increase_count = static_cast<DWORD>(max_object_count_ - chunks_.size());
		if (increase_count > variance_object_count_)
			increase_count = variance_object_count_;

		for (DWORD i = 0; i < increase_count; ++i)
		{
			_Ty* object = new _Ty;
			chunks_.insert(ChunkStatusMap::value_type(object, OBJECT_STATUS::OBJECT_STATUS_IDLE));
			chunk_que_.insert(object);
		}
	}

	return true;
}

template<typename _Ty>
bool ObjectPool<_Ty>::DecreasePool()
{
	std::unique_lock<std::mutex> lock(mtx_);
	if (chunk_que_.size() <= min_idle_object_count_)
		return false;

	DWORD decrease_count = static_cast<DWORD>(chunk_que_.size() - min_idle_object_count_);
	if (decrease_count > variance_object_count_)
		decrease_count = variance_object_count_;

	for (DWORD i = 0; i < decrease_count; ++i)
	{
		if (chunk_que_.empty())
			break;

		_Ty* object = *chunk_que_.pop();
		auto itor = chunks_.find(object);

		if (itor == chunks_.end())
		{
			assert(false);
			break;
		}

		if (object.use_count() != min_idle_object_count_)
		{
			assert(false);
			break;
		}

		chunks_.erase(object);

		delete object;
	}

	return true;
}