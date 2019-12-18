#pragma once
#include "stdafx.h"
#include "object.h"

template<typename _Ty>
using is_object = std::enable_if<std::is_base_of<object, _Ty>::value, _Ty>;

template<typename _Ty, typename is_object<_Ty>::type * = nullptr>
class ObjectPool final
{
public:
	using Chunks = std::queue<_Ty*>;
	using ActiveObjects = std::set<_Ty*>;

	ObjectPool(DWORD min_count = 0, DWORD max_count = 0, DWORD extend_count = 0)
		: min_object_count_(min_count), max_object_count_(max_count), extend_object_count_(extend_count)
	{
		IncreasePool(min_object_count_);
	}

	~ObjectPool()
	{
		min_object_count_ = 0;
		Clear();
	}

	ObjectPool(const ObjectPool<_Ty>&) = delete;
	void operator=(const ObjectPool<_Ty>&) = delete;

	bool Push(_Ty*& object)
	{
		std::unique_lock<std::mutex> lock(mtx_);
		{
			if (active_objects_.find(object) == active_objects_.end())
			{
				return false;
			}

			active_objects_.erase(object);
			chunks_.push(object);
		}

		object = nullptr;

		return true;
	}

	_Ty* Pop()
	{
		_Ty* object = nullptr;
		std::unique_lock<std::mutex> lock(mtx_);
		{
			if (chunks_.empty())
			{
				if (IncreasePool() == false)
					return object;
			}

			object = chunks_.front();
			if (active_objects_.insert(object).second == false)
			{
				return nullptr;
			}

			object->initilize();
			chunks_.pop();

			
		}

		return object;
	}

private:
	
	DWORD GetIdleChunkSize() { return static_cast<DWORD>(chunks_.size()); }
	DWORD GetActiveChunkSize() { return abs(static_cast<DWORD>(chunks_.size() - active_objects_.size())); }
	DWORD GetTotalChunkSize() { return static_cast<DWORD>(chunks_.size() + active_objects_.size()); }

	void Clear()
	{
		std::for_each(active_objects_.begin(), active_objects_.end(),
			[&](_Ty* object)
		{
			chunks_.push(object);
		});

		active_objects_.clear();
		if (DecreasePool(GetTotalChunkSize()) == false)
		{
			//...
		}
	}

	bool IncreasePool(DWORD extend_size = 0)
	{
		if (extend_size == 0)
			extend_size = extend_object_count_;

		std::unique_lock<std::mutex> lock(mtx_);
		{
			if (GetIdleChunkSize() >= max_object_count_)
				return false;

			if (GetIdleChunkSize() + extend_size > max_object_count_)
				extend_size = max_object_count_ - GetIdleChunkSize();

			for (DWORD i = 0; i < extend_size; ++i)
			{
				_Ty* object = new _Ty;
				chunks_.push(object);
			}
		}

		return true;
	}

	bool DecreasePool(DWORD reduce_size = 0)
	{
		if (reduce_size == 0)
			reduce_size = extend_object_count_;

		std::unique_lock<std::mutex> lock(mtx_);
		{
			if (GetIdleChunkSize() <= min_object_count_)
				return false;

			if (GetIdleChunkSize() - reduce_size < min_object_count_)
				reduce_size = GetIdleChunkSize() - reduce_size;

			for (DWORD i = 0; i < reduce_size; ++i)
			{
				if (chunks_.empty())
					break;

				_Ty* object = std::move(chunks_.front());
				delete object;
				chunks_.pop();
			}
		}

		return true;
	}

	DWORD min_object_count_;
	DWORD max_object_count_;
	DWORD extend_object_count_;

	Chunks chunks_;
	ActiveObjects active_objects_;

	std::mutex mtx_;
};