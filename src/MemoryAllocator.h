#pragma once
#include "stdafx.h"
#include "MemoryPool.h"

#ifdef _DEBUG
#pragma comment(lib, "dbghelp.lib")

const static size_t _default_stack_depth_ = 32;
using CallStack = std::vector<PVOID>;

struct CallStackTrace
{
	CallStackTrace(const std::string type_name = "", const std::size_t alloc_size = 0, const std::vector<PVOID>& callstack = std::vector<PVOID>())
		: type_name_(type_name), alloc_size_(alloc_size), callstack_(callstack)
	{}

	const std::string type_name_;
	const std::size_t alloc_size_;	
	const CallStack callstack_;
};

class CallStackTraceManager
{
public:
	CallStackTraceManager() = default;
	~CallStackTraceManager() = default;

	template<typename T>
	void Add(const PVOID ptr, const CallStack& callstack)
	{
		assert(callstack_traces_.try_emplace(ptr, typeid(T).name(), sizeof(T), callstack).second);
	}

	void Remove(const PVOID ptr)
	{
		auto itor = callstack_traces_.find(ptr);
		if (itor == callstack_traces_.end())
		{
			assert(false);
			return;
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
class MemoryAllocator
{
public:
	using value_type = T;
	using pointer = T*;
	using const_pointer = const T*;
	using void_pointer = void*;
	using const_void_pointer = const void*;
	using differenceTpe = std::ptrdiff_t;
	using sizeTpe = std::size_t;

	template<typename U>
	struct rebind {
		using other = MemoryAllocator<U>;
	};

	MemoryAllocator() noexcept
		: mem_pool_(std::make_shared<MemoryPool>())
#ifdef _DEBUG
		, callstack_trace_manager_(std::make_shared<CallStackTraceManager>())
#endif
	{
		assert(mem_pool_ != nullptr);
#ifdef _DEBUG
		assert(callstack_trace_manager_ != nullptr);
#endif
	}
	
	MemoryAllocator<T>(const MemoryAllocator<T>& other) noexcept
		: mem_pool_(other.mem_pool_)
#ifdef _DEBUG
		, callstack_trace_manager_(other.callstack_trace_manager_)
#endif
	{
		assert(mem_pool_ != nullptr);
#ifdef _DEBUG
		assert(callstack_trace_manager_ != nullptr);
#endif
	}

	template<typename U> 
	friend class MemoryAllocator;

	template<typename U>
	MemoryAllocator(const MemoryAllocator<U>& other) noexcept
		: mem_pool_(other.mem_pool_)
#ifdef _DEBUG
		, callstack_trace_manager_(other.callstack_trace_manager_)
#endif
	{
		assert(mem_pool_ != nullptr);
#ifdef _DEBUG
		assert(callstack_trace_manager_ != nullptr);
#endif
	}

	MemoryAllocator<T>& operator=(const MemoryAllocator<T>& other) noexcept
	{
		if (this != &other)
		{
			mem_pool_ = other.mem_pool_;
#ifdef _DEBUG
			callstack_trace_manager_ = other.callstack_trace_manager_;
#endif
		}

		return *this;
	}

	T* allocate(const size_t size)
	{
		T* alloc_ptr = mem_pool_->alloc<T>(size);

#ifdef _DEBUG
		DWORD hash;
		CallStack callstack(_default_stack_depth_);
		CaptureStackBackTrace(0, static_cast<DWORD>(_default_stack_depth_), callstack.data(), &hash);

		callstack_trace_manager_->Add<T>(alloc_ptr, callstack);
#endif

		return alloc_ptr;
	}

	void deallocate(T* const ptr, const size_t size) noexcept {

#ifdef _DEBUG
		callstack_trace_manager_->Remove(ptr);
#endif
		mem_pool_->free<T>(ptr, size);
	}

	template<typename T, typename... Args>
	void construct(T* ptr, Args&&... args)
	{
		if (ptr == nullptr)
			throw std::bad_alloc();

		new(ptr) T(std::forward<Args>(args)...);
	}

	template<typename T>
	void destroy(T* ptr) noexcept
	{
		ptr->~T();
	}

#ifdef _DEBUG
	std::shared_ptr<CallStackTraceManager> callstack_trace_manager_;
#endif

	std::shared_ptr<MemoryPool> mem_pool_;
};