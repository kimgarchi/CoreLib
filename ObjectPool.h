#pragma once
#include "stdafx.h"
#include "object.h"

class ObjectPoolBase abstract
{
protected:
	friend class ObjectStation;
	virtual void Clear() abstract;
};

template<typename _Ty, typename is_object<_Ty>::type * = nullptr>
class ObjectPool final : public ObjectPoolBase
{
public:
	using Chunks = std::queue<_Ty*>;
	using ActiveObjects = std::set<_Ty*>;

	ObjectPool(DWORD init_count, DWORD max_count, DWORD extend_count)
		: init_object_count_(init_count), max_object_count_(max_count), extend_object_count_(extend_count)
	{
		IncreasePool(init_object_count_);
	}

	ObjectPool(const ObjectPool<_Ty>&) = delete;
	void operator=(const ObjectPool<_Ty>&) = delete;

	bool Push(_Ty*& object)
	{
		object->initilize();

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

			chunks_.pop();
		}

		return object;
	}

	void set_extend_object_count(DWORD extend_count) { extend_object_count_ = extend_count; }	
	bool set_max_object_count(DWORD max_count) { max_object_count_ = max_count; }

private:	
	virtual void Clear() override
	{
		std::for_each(active_objects_.begin(), active_objects_.end(),
			[&](_Ty* object)
		{
			chunks_.push(object);
		});

		active_objects_.clear();
		if (DecreasePool(GetTotalChunkSize()) == false)
		{
			//... ?
		}
	}

	DWORD GetIdleChunkSize() { return static_cast<DWORD>(chunks_.size()); }
	DWORD GetActiveChunkSize() { return abs(static_cast<DWORD>(chunks_.size() - active_objects_.size())); }
	DWORD GetTotalChunkSize() { return static_cast<DWORD>(chunks_.size() + active_objects_.size()); }

	bool IncreasePool(DWORD extend_size = 0)
	{
		if (extend_size == 0)
			extend_size = extend_object_count_;

		if (GetTotalChunkSize() >= max_object_count_)
			return false;

		if (GetIdleChunkSize() + extend_size > max_object_count_)
			extend_size = max_object_count_ - GetIdleChunkSize();

		for (DWORD i = 0; i < extend_size; ++i)
		{
			_Ty* object = new _Ty;
			chunks_.push(object);
		}

		return true;
	}

	bool DecreasePool(DWORD reduce_size = 0)
	{
		if (reduce_size == 0)
			reduce_size = extend_object_count_;

		if (GetIdleChunkSize() - reduce_size < init_object_count_)
			reduce_size = GetIdleChunkSize() - reduce_size;

		for (DWORD i = 0; i < reduce_size; ++i)
		{
			if (chunks_.empty())
				break;

			_Ty* object = std::move(chunks_.front());
			delete object;
			chunks_.pop();
		}

		return true;
	}

	DWORD init_object_count_;
	DWORD max_object_count_;
	DWORD extend_object_count_;

	Chunks chunks_;
	ActiveObjects active_objects_;

	std::mutex mtx_;
};