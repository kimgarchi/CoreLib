#pragma once
#include "stdafx.h"

#define MEGA_BYTE_TO_BYTE 1048576

template<typename _Ty>
class ObjectChunk final
{
private:
	using Location = size_t;
	using MemQue = std::queue<void*>;
	using AllocMems = std::map<void*, Location>;

public:
	ObjectChunk(size_t obj_cnt)
		: obj_cnt_(obj_cnt), m_alloc_ptr_(nullptr), alloc_size_(sizeof(_Ty)* obj_cnt)
	{
		m_alloc_ptr_ = alloc_size_ >= MEGA_BYTE_TO_BYTE ?
			HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, alloc_size_) : std::malloc(alloc_size_);

		if (m_alloc_ptr_ == nullptr)
			throw std::runtime_error("malloc failed");

		size_t forward_step = 0;
		for (auto idx = 0; idx < obj_cnt_; ++idx)
		{
			auto ptr = static_cast<_Ty*>(m_alloc_ptr_) + idx;
			alloc_mems_.emplace(ptr, idx);
			mem_que_.emplace(ptr);
		}
	}

	~ObjectChunk()
	{
		while (!mem_que_.empty()) mem_que_.pop();

		alloc_mems_.clear();

		if (alloc_size_ >= MEGA_BYTE_TO_BYTE)
			HeapFree(GetProcessHeap(), NULL, m_alloc_ptr_);
		else
			std::free(m_alloc_ptr_);
		m_alloc_ptr_ = nullptr;
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
			throw std::runtime_error("mem que nullptr");

		_Ty* object = new (ptr)_Ty(std::forward<_Tys>(Args)...);
		if (object == nullptr)
			throw std::runtime_error("new failed");

		mem_que_.pop();

		return object;
	}

	bool Push(void* ptr)
	{
		if (alloc_mems_.find(ptr) == alloc_mems_.end())
			return false;

		delete (_Ty*)ptr;
		mem_que_.emplace(ptr);

		return true;
	}

private:
	size_t obj_cnt_;
	size_t alloc_size_;
	void* const m_alloc_ptr_;

	AllocMems alloc_mems_;
	MemQue mem_que_;
};