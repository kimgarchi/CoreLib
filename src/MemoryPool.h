#pragma once
#include "stdafx.h"
#include "MemoryChunk.h"

#pragma warning (push)
#pragma warning (disable : 4291)
#pragma warning (disable : 4348)
#pragma warning (disable : 6305)

#ifdef _DEBUG
const static size_t _default_max_alloc_cnt_ = 100;
#else
const static size_t _default_max_alloc_cnt_ = 1000;
#endif

class MemoryPool final
{
public:
	using MemChunks = std::list<std::shared_ptr<MemoryChunk>>;

	MemoryPool() = default;
	MemoryPool(const MemoryPool& other) noexcept
		: mem_chunks_(other.mem_chunks_)
	{
	}

	MemoryPool& operator=(const MemoryPool& other) noexcept
	{
		if (this != &other)
		{
			mem_chunks_ = other.mem_chunks_;
		}

		return *this;
	}

	~MemoryPool() = default;

	template<typename T>
	T* alloc(const std::size_t size)
	{
		std::shared_ptr<MemoryChunk> mem_chunk = FindAllocableChunk<T>(size);
		if (mem_chunk == nullptr)
		{
			assert(false);
			return nullptr;
		}
		
		return static_cast<T*>(mem_chunk->alloc(size));
	}

	template<typename T>
	bool free(T* ptr, std::size_t size)
	{
		std::shared_ptr<MemoryChunk> mem_chunk = FindHasPtrMemoryChunk(ptr);
		if (mem_chunk == nullptr)
		{
			/// error
			assert(false);
			return false;
		}
		
		return mem_chunk->free<T>(ptr, size);
	}

private:
	template<typename T>
	std::shared_ptr<MemoryChunk> allocChunk(const std::size_t size)
	{
		const std::size_t block_size = sizeof(T);
		const std::size_t alloc_size = max(size, _default_max_alloc_cnt_);

		std::shared_ptr<MemoryChunk> alloc_mem_chunk = std::make_shared<MemoryChunk>(block_size, alloc_size);
		if (alloc_mem_chunk == nullptr)
			throw std::bad_alloc();

		mem_chunks_.emplace_back(alloc_mem_chunk);
			
		return alloc_mem_chunk;
	}

	template<typename T>
	std::shared_ptr<MemoryChunk> FindAllocableChunk(const std::size_t size)
	{
		for (std::shared_ptr<MemoryChunk> mem_chunk : mem_chunks_)
		{
			if (mem_chunk->isAllocable(size) == false)
				continue;

			return mem_chunk;
		}
		
		/// noti info log (alloc_size != size)
		return allocChunk<T>(size);
	}

	std::shared_ptr<MemoryChunk> FindHasPtrMemoryChunk(PVOID ptr)
	{
		for (std::shared_ptr<MemoryChunk> mem_chunk : mem_chunks_)
		{
			if (mem_chunk->hasPtr(ptr) == false)
				continue;

			return mem_chunk;
		}

		return nullptr;
	}

	MemChunks mem_chunks_;
};