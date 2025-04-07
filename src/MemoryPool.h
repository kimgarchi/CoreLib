#pragma once
#include "stdafx.h"
#include "MemoryChunk.h"
#include "LockObject.h"

#pragma warning (push)
#pragma warning (disable : 4291)
#pragma warning (disable : 4348)
#pragma warning (disable : 6305)

#ifdef _DEBUG
const static std::size_t _default_max_alloc_cnt_ = 100;
#else
const static std::size_t _default_max_alloc_cnt_ = 1000;
#endif

#ifdef _DEBUG
#pragma comment(lib, "dbghelp.lib")

const static size_t _default_stack_depth_ = 32;
using CallStack = std::vector<PVOID>;

struct CallStackTrace
{
	const std::string type_name_ = "";
	const std::size_t alloc_size_ = 0;
	const std::size_t alloc_count_ = 0;
	const std::thread::id alloc_thread_id;
	const CallStack callstack_;
};

class CallStackTraceManager
{
public:
	CallStackTraceManager() = default;
	~CallStackTraceManager() = default;

	template<typename T>
	void Add(const PVOID ptr, const std::size_t alloc_count, const CallStack& callstack)
	{
		assert(callstack_traces_.try_emplace(ptr, CallStackTrace{ typeid(T).name(), sizeof(T), alloc_count, std::this_thread::get_id(), callstack}).second);
	}

	void Remove(const PVOID ptr)
	{
		auto itor = callstack_traces_.find(ptr);
		if (itor == callstack_traces_.end())
		{
			assert(false);
			return;
		}

		const std::thread::id dealloc_thread_id = std::this_thread::get_id();
		if (dealloc_thread_id != itor->second.alloc_thread_id)
		{
			/// trace
		}

		callstack_traces_.erase(ptr);
	}

	CallStackTrace getCallStack(PVOID ptr)
	{
		auto itor = callstack_traces_.find(ptr);
		if (itor == callstack_traces_.end())
		{
			assert(false);
			return CallStackTrace();
		}

		return itor->second;
	}

private:
	std::map<const PVOID, const CallStackTrace> callstack_traces_;
};
#endif

template<typename T>
class MemoryPool final
{
public:
	using MemChunks = std::list<std::shared_ptr<MemoryChunk<T>>>;

	MemoryPool() = default;
	MemoryPool(const MemoryPool&) = delete;
	void operator=(const MemoryPool&) = delete;

	~MemoryPool() = default;

	T* alloc(const std::size_t size)
	{
		SingleLock sl(sync_mutex_);

		const std::size_t block_size = sizeof(T);
		std::shared_ptr<MemoryChunk<T>> mem_chunk = FindAllocableChunk(size);
		if (mem_chunk == nullptr)
		{
			assert(false);
			return nullptr;
		}
		
#ifdef _DEBUG
		T* alloc_ptr = static_cast<T*>(mem_chunk->alloc(size));
		if (alloc_ptr == nullptr)
			return nullptr;

		DWORD hash;
		CallStack callstack(_default_stack_depth_);
		CaptureStackBackTrace(0, static_cast<DWORD>(_default_stack_depth_), callstack.data(), &hash);

		callstack_trace_manager_.Add<T>( alloc_ptr, size, callstack);

		return alloc_ptr;
#else
		return static_cast<T*>(mem_chunk->alloc(size));
#endif
	}

	bool free(T* ptr, std::size_t size)
	{
		SingleLock sl(sync_mutex_);

		std::shared_ptr<MemoryChunk<T>> mem_chunk = FindHasPtrMemoryChunk(ptr);
		if (mem_chunk == nullptr)
		{
			/// error
			assert(false);
			return false;
		}
		
#ifdef _DEBUG
		callstack_trace_manager_.Remove(ptr);
#endif
		return mem_chunk->free(ptr, size);
	}

private:
	std::shared_ptr<MemoryChunk<T>> allocChunk(const std::size_t size)
	{
		const std::size_t alloc_count = max(size, _default_max_alloc_cnt_);

		std::shared_ptr<MemoryChunk<T>> alloc_mem_chunk = std::make_shared<MemoryChunk<T>>(alloc_count);
		if (alloc_mem_chunk == nullptr)
			throw std::bad_alloc();

		mem_chunks_.emplace_back(alloc_mem_chunk);
			
		return alloc_mem_chunk;
	}

	std::shared_ptr<MemoryChunk<T>> FindAllocableChunk(const std::size_t size)
	{
		for (std::shared_ptr<MemoryChunk<T>> mem_chunk : mem_chunks_)
		{
			if (mem_chunk->isAllocable(size) == false)
				continue;

			return mem_chunk;
		}
		
		/// noti info log (alloc_size != size)
		return allocChunk(size);
	}

	std::shared_ptr<MemoryChunk<T>> FindHasPtrMemoryChunk(PVOID ptr)
	{
		for (std::shared_ptr<MemoryChunk<T>> mem_chunk : mem_chunks_)
		{
			if (mem_chunk->hasPtr(ptr) == false)
				continue;

			return mem_chunk;
		}

		return nullptr;
	}

#ifdef _DEBUG
	CallStackTraceManager callstack_trace_manager_;
#endif
	SyncMutex sync_mutex_;
	MemChunks mem_chunks_;
};