#pragma once
#include "stdafx.h"
#include "object.h"

#pragma warning (push)
#pragma warning (disable : 4291)
#pragma warning (disable : 4348)
#pragma warning (disable : 6305)

#define TID					0
#define ALLOC_ID			1
#define INVALID_ALLOC_ID	0
#define INIT_ALLOC_ID		1

class ObjectPoolBase abstract
{
protected:
	friend class ObjectStation;
	virtual void Clear() abstract;
};

template<typename _Ty, is_object<_Ty> = nullptr>
class ObjectPool final : public ObjectPoolBase
{
private:	
	class SegmentPool final
	{
	private:
		using Location = size_t;
		using MemQue = std::queue<void*>;
		using AllocMems = std::map<void*, Location>;

	public:
		SegmentPool(size_t obj_cnt)
			: obj_cnt_(obj_cnt), m_ptr_(nullptr)
		{
			m_ptr_ = std::malloc(sizeof(_Ty) * obj_cnt_);
			if (m_ptr_ == nullptr)
				std::bad_alloc{};

			auto size = sizeof(_Ty);
			size_t forward_step = 0;
			for (auto idx = 0; idx < obj_cnt_; ++idx)
			{
				auto ptr = (static_cast<_Ty*>(m_ptr_) + forward_step);

				forward_step += size;
				alloc_mems_.emplace(ptr, idx);
				mem_que_.emplace(ptr);
			}
		}

		~SegmentPool()
		{
			while (!mem_que_.empty()) mem_que_.pop();

			alloc_mems_.clear();
			std::free(m_ptr_);
		}

		bool UnusedPool() { return (mem_que_.size() == obj_cnt_) ? true : false; }
		bool Empty() { return mem_que_.empty(); }

		template<typename ..._Tys>
		_Ty* Pop(_Tys&&... Args)
		{
			if (mem_que_.empty())
				return nullptr;

			void* ptr = mem_que_.front();
			if (ptr == nullptr)
				std::bad_alloc{};

			_Ty* object = new (ptr)_Ty(std::forward<_Tys>(Args)...);
			if (object == nullptr)
				std::bad_alloc{};

			mem_que_.pop();

			return object;
		}

		bool Push(_Ty*& object)
		{
			if (alloc_mems_.find(object) == alloc_mems_.end())
				return false;

			delete object;
			mem_que_.emplace(object);
			
			return true;
		}

	private:
		size_t obj_cnt_;
		void* m_ptr_;

		AllocMems alloc_mems_;
		MemQue mem_que_;
	};
	
	using ObjectCount = std::atomic_size_t;
	using Chunks = std::map<AllocID, SegmentPool>;
	using Objects = std::map<_Ty*, AllocID>;

public:
	ObjectPool(const ObjectPool<_Ty>&) = delete;
	void operator=(const ObjectPool<_Ty>&) = delete;

	~ObjectPool()
	{
		assert(alloc_objects_.empty());
	}

	bool Push(_Ty*& object)
	{
		AllocID alloc_id = GetAllocID(object);
		if (alloc_id == INVALID_ALLOC_ID)
			return false;

		if (Push(alloc_id, object) == false)
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
			SegmentPool& segment_pool = chunk.second;
			if (segment_pool.Empty())
				continue;

			alloc_id = chunk.first;
			object = segment_pool.Pop(Args...);
		}

		if (alloc_id == INVALID_ALLOC_ID || object == nullptr)
			return nullptr;

		if (object != nullptr)
			alloc_objects_.emplace(object, alloc_id);
		else
			assert(false);

		sub_object_count(1);
		
		TryExtend();

		return object;		
	}

private:
	friend class ObjectStation;

	ObjectPool(size_t segment_object_cnt, float extend_remain_rate)
		: assign_alloc_id_(INIT_ALLOC_ID), segment_object_cnt_(segment_object_cnt), extend_remain_rate_(extend_remain_rate)
	{
		AllocChunk();
	}

	AllocID AssignAllocID() { return assign_alloc_id_++; }

	virtual void Clear() override
	{
		while (alloc_objects_.empty() == false)
		{
			auto itor = alloc_objects_.begin();
			if (Push(itor->second, itor->first) == false)
				std::bad_function_call{};
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
			// temp
			std::cout << excp.what() << std::endl;
			return;
		}
		catch (std::bad_function_call & excp)
		{
			// temp
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

	bool Push(AllocID alloc_id, _Ty* object)
	{
		auto itor = chunks_.find(alloc_id);
		if (itor == chunks_.end())
			return false;

		SegmentPool& segment_pool = itor->second;
		if (segment_pool.Push(object) == false)
			return false;

		alloc_objects_.erase(object);
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

	AllocID GetAllocID(_Ty* object)
	{
		auto itor = alloc_objects_.find(object);
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
	Chunks chunks_;
	Objects alloc_objects_;
};

#pragma warning (pop)