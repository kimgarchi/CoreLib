#pragma once
#include "stdafx.h"
#include "ObjectChunk.h"

#pragma warning (push)
#pragma warning (disable : 4291)
#pragma warning (disable : 4348)
#pragma warning (disable : 6305)

#define ALLOC_ID 1
#define INVALID_ALLOC_ID 0
#define INIT_ALLOC_ID 1

class MemoryPoolBase abstract
{
protected:
	friend class MemoryManager;

	virtual bool Push(void* ptr) abstract;
	virtual void Clear() abstract;
};

template<typename _Ty>
class ObjectPool final : public MemoryPoolBase
{
public:
	using ObjectCount = std::atomic_size_t;
	using ObjectChunks = std::unordered_map<AllocID, ObjectChunk>;
	using Objects = std::map<PVOID, AllocID>;

	ObjectPool(size_t segment_object_cnt, float extend_remain_rate)
		: assign_alloc_id_(INIT_ALLOC_ID), segment_object_cnt_(segment_object_cnt), extend_remain_rate_(extend_remain_rate)
	{
		AllocChunk();
	}

	ObjectPool(const ObjectPool<_Ty>&) = delete;
	void operator=(const ObjectPool<_Ty>&) = delete;

	~ObjectPool()
	{
		assert(alloc_objects_.empty());
	}

	virtual bool Push(void* ptr) override
	{
		AllocID alloc_id = GetAllocID(ptr);
		if (alloc_id == INVALID_ALLOC_ID)
			return false;

		if (Push(alloc_id, ptr) == false)
			return false;

		add_object_count(1);

		return true;
	}

	template<typename ..._Tys>
	_Ty* Pop(_Tys&&... Args)
	{
		AllocID alloc_id = INVALID_ALLOC_ID;
		_Ty* object = nullptr;

		for (auto& chunk : chunks_)
		{
			ObjectChunk& memory_chunk = chunk.second;
			if (memory_chunk.Empty())
				continue;

			alloc_id = chunk.first;
			object = memory_chunk.Pop(Args...);
		}

		if (alloc_id == INVALID_ALLOC_ID || object == nullptr)
			return nullptr;

		sub_object_count(1);

		if (object != nullptr)
		{
			assert(alloc_objects_.emplace(object, alloc_id).second);
			return object;
		}
		
		TryExtend();

		return Pop(Args...);
	}

private:	
	AllocID AssignAllocID() { return assign_alloc_id_++; }

	virtual void Clear() override
	{
		while (alloc_objects_.empty() == false)
		{
			auto itor = alloc_objects_.begin();
			if (Push(itor->second, itor->first) == false)
				throw std::runtime_error("mem que clear failed");
		}
		
		chunks_.clear();
	}

	void AllocChunk()
	{
		try
		{
			chunks_.emplace(AssignAllocID(), segment_object_cnt_);
		}
		catch (std::bad_alloc & excp)
		{
			std::cout << excp.what() << std::endl;
			return;
		}
		catch (std::bad_function_call & excp)
		{
			std::cout << excp.what() << std::endl;
			return;
		}
		
		add_object_count(segment_object_cnt_);
	}

	void TryExtend()
	{
		if (object_count() / chunks_.size() * segment_object_cnt_ > extend_remain_rate_)
			return;

		AllocChunk();
	}

	bool Push(AllocID alloc_id, void* ptr)
	{
		auto itor = chunks_.find(alloc_id);
		if (itor == chunks_.end())
			return false;

		ObjectChunk& segment_pool = itor->second;
		if (segment_pool.Push(ptr) == false)
			return false;

		alloc_objects_.erase(ptr);
		return true;
	}

	bool DeAllocChunk(AllocID alloc_id)
	{
		// ...?
		return true;
	}

	bool IsAllocObject(_Ty* object)
	{
		if (alloc_objects_.find(object) == alloc_objects_.end())
			return false;

		return true; 
	}

	AllocID GetAllocID(void* ptr)
	{
		auto itor = alloc_objects_.find(ptr);
		if (itor == alloc_objects_.end())
			return INVALID_ALLOC_ID;

		return itor->second;
	}

	void add_object_count(size_t count) { object_count_.fetch_add(count); }
	void sub_object_count(size_t count) { object_count_.fetch_sub(count); }
	const ObjectCount& object_count() { return object_count_; }

	const size_t segment_object_cnt_;
	const float extend_remain_rate_;
	ObjectCount object_count_;

	AllocID assign_alloc_id_;
	ObjectChunks chunks_;
	Objects alloc_objects_;
};