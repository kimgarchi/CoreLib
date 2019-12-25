#pragma once
#include "stdafx.h"
#include "object.h"

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

template<typename _Ty, typename is_object<_Ty>::type * = nullptr>
class ObjectPool final : public ObjectPoolBase
{
private:
	class SegmentPool;
	using AllocID = size_t;
	using Chunks = std::map<AllocID, SegmentPool>;
	using ChunkElement = std::pair<AllocID, SegmentPool>;
	using Objects = std::map<_Ty*, AllocID>;
	using ObjectElement = std::pair<_Ty*, AllocID>;

	class SegmentPool final
	{
	private:
		enum class STATE
		{
			IDLE,
			USE,
		};

		using ObjectQue = std::queue<_Ty*>;
		using ObjectByState = std::map<_Ty*, STATE>;

		size_t obj_cnt_;
		void* m_ptr_;		
		ObjectQue object_que_;
		ObjectByState object_by_state_;

	public:
		SegmentPool(size_t obj_cnt)
			: obj_cnt_(obj_cnt), m_ptr_(nullptr)
		{
			m_ptr_ = std::malloc(sizeof(_Ty) * obj_cnt_);
			if (m_ptr_ == nullptr)
				std::bad_alloc{};

			char* ptr = static_cast<char*>(m_ptr_);
			for (size_t idx = 0; idx < obj_cnt_; ++idx)
			{
				size_t forward_step = idx * sizeof(_Ty);
				_Ty* data = new(ptr + forward_step) _Ty;
				if (data == nullptr)
					std::bad_function_call{};

				object_que_.push(data);
				object_by_state_.insert(ObjectByState::value_type(data, STATE::IDLE));
			}
		}

		~SegmentPool()
		{
			std::cout << "delete" << std::endl;
			//assert(m_ptr_ == nullptr);
		}

		void Clear()
		{
			std::swap(ObjectQue(), object_que_);
			object_by_state_.clear();
			std::free(m_ptr_);
		}

		bool UnusedPool() { return (object_que_.size() == obj_cnt_) ? true : false; }
		bool Empty() { return object_que_.empty(); }

		_Ty* Pop()
		{
			if (object_que_.empty())
				return nullptr;

			_Ty* object = object_que_.front();
			if (ChangeState(object, STATE::USE) == false)
				return nullptr;

			object_que_.pop();
			object->initilize();

			return object;
		}

		bool Push(_Ty*& object)
		{
			if (ChangeState(object, STATE::IDLE) == false)
				return false;

			object_que_.push(object);

			return true;
		}

	private:
		bool ChangeState(_Ty* object, STATE state)
		{
			auto itor = object_by_state_.find(object);
			if (itor == object_by_state_.end())
				return false;

			STATE& cur_state = itor->second;
			if (cur_state == state)
				return false;

			cur_state = state;

			return true;
		}
	};
	
public:
	ObjectPool(size_t segment_object_cnt)
		: assign_alloc_id_(INIT_ALLOC_ID), segment_object_cnt_(segment_object_cnt)
	{
		AllocChunk();
	}

	ObjectPool(const ObjectPool<_Ty>&) = delete;
	void operator=(const ObjectPool<_Ty>&) = delete;

	bool Push(_Ty*& object)
	{
		AllocID alloc_id = GetAllocID(object);
		if (alloc_id == INVALID_ALLOC_ID)
			return false;

		return Push(alloc_id, object);
	}

	_Ty* Pop()
	{
		AllocID alloc_id = INVALID_ALLOC_ID;
		_Ty* object = nullptr;

		for (auto & chunk : chunks_)
		{
			SegmentPool& segment_pool = chunk.second;
			if (segment_pool.Empty())
				continue;

			alloc_id = chunk.first;
			object = segment_pool.Pop();
		}

		if (alloc_id == INVALID_ALLOC_ID ||
			object == nullptr)
		{
			if (AllocChunk() == false)
				return nullptr;

			return Pop();
		}

		return object;
	}

private:
	AllocID AssignAllocID() { return assign_alloc_id_++; }

	virtual void Clear() override
	{
		alloc_objects_.clear();
	}

	bool AllocChunk()
	{
		AllocID alloc_id = AssignAllocID();
		chunks_.emplace(ChunkElement(alloc_id, std::forward<SegmentPool>(SegmentPool(segment_object_cnt_))));

		/*
		try
		{
			
		}
		catch (std::bad_alloc& exception)
		{
			// ...
			return false;
		}
		*/
		return true;
	}

	bool Push(AllocID alloc_id, _Ty*& object)
	{
		auto itor = chunks_.find(alloc_id);
		if (itor == chunks_.end())
			return false;

		SegmentPool& segment_pool = itor->second;
		return segment_pool.Push(object);
	}

	bool DeAllocChunk()
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

	const size_t segment_object_cnt_;
	AllocID assign_alloc_id_;
	Chunks chunks_;
	Objects alloc_objects_;
	std::mutex mtx_;
};