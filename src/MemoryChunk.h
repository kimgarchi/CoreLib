#pragma once
#include "stdafx.h"
#include "BitSet.h"
#include <new>

#define MEGA_BYTE_TO_BYTE 1048576

class MemoryChunk final
{
public:
	MemoryChunk(const std::size_t block_size, const size_t block_count)
		: block_size_(block_size), block_count_(block_count), 
		m_alloc_begin_ptr_(nullptr), m_alloc_end_ptr_(nullptr), m_alloc_bitset_(block_count)
	{
		const std::size_t alloc_size = block_size * block_count;

		m_alloc_begin_ptr_ = alloc_size >= MEGA_BYTE_TO_BYTE ?
			HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, alloc_size) : std::malloc(alloc_size);

		if (m_alloc_begin_ptr_ == nullptr)
			throw std::runtime_error("malloc failed");

		m_alloc_end_ptr_ = static_cast<BYTE*>(m_alloc_begin_ptr_) + alloc_size;
	}

	MemoryChunk(const MemoryChunk&) = delete;
	void operator=(const MemoryChunk&) = delete;

	~MemoryChunk()
	{
		for (std::size_t pos = 0; pos < block_count_; ++pos)
		{
			if (m_alloc_bitset_.getBit(pos))
			{
				/// error log
			}
		}

		if (block_count_ >= MEGA_BYTE_TO_BYTE)
			HeapFree(GetProcessHeap(), NULL, m_alloc_begin_ptr_);
		else
			std::free(m_alloc_begin_ptr_);

		m_alloc_begin_ptr_ = nullptr;
		m_alloc_end_ptr_ = nullptr;
	}

	PVOID alloc(const std::size_t size)
	{
		const std::size_t alloc_pos = m_alloc_bitset_.getRelaySizePos(size, false);
		if (alloc_pos == -1)
			return nullptr;

		PVOID alloc_ptr = getPtr(alloc_pos);
		if (alloc_ptr == nullptr)
			return nullptr;

		m_alloc_bitset_.setBit(alloc_pos, alloc_pos + (size - 1), true);

		return alloc_ptr;
	}

	template<typename T>
	bool free(PVOID ptr, const std::size_t size)
	{
		if (ptr == nullptr)
		{
			return false;
		}

		if (hasPtr(ptr) == false)
		{
			return false;
		}	

		std::size_t begin_pos = getPtrPos(ptr);
		std::size_t end_pos = begin_pos + (size - 1);

		if (begin_pos < 0 || end_pos >= block_count_)
			return false;

		m_alloc_bitset_.setBit(begin_pos, end_pos, false);
		
		for (auto pos = begin_pos; pos <= end_pos; ++pos)
			static_cast<T*>(getPtr(pos))->~T();

		return true;
	}

	inline bool isAllocable(const std::size_t size) const
	{
		return m_alloc_bitset_.getRelaySizePos(size, false) != -1;
	}

	bool hasPtr(PVOID ptr)
	{
		return m_alloc_begin_ptr_ <= ptr && m_alloc_end_ptr_ >= ptr;
	}

private:
	std::size_t getPtrPos(PVOID ptr) const
	{
		return (static_cast<BYTE*>(ptr) - static_cast<BYTE*>(m_alloc_begin_ptr_)) / block_size_;
	}

	PVOID getPtr(std::size_t pos) const
	{
		return static_cast<BYTE*>(m_alloc_begin_ptr_) + (pos * block_size_);
	}

	const std::size_t block_size_;
	const std::size_t block_count_;

	PVOID m_alloc_begin_ptr_;
	PVOID m_alloc_end_ptr_;
	BitSet m_alloc_bitset_;
};