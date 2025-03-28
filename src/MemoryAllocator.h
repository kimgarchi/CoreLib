#pragma once
#include "stdafx.h"
#include "MemoryPool.h"

#pragma once
#include "stdafx.h"
#include "MemoryAllocator.h"

#ifndef MEMORY_MANAGER_H_INCLUDED
#define MEMORY_MANAGER_H_INCLUDED

template<typename T>
class MemoryAllocator;

class MemoryManager final
{
private:
	using MemoryAllocators = std::unordered_map<std::type_index, std::shared_ptr<VOID>>;

public:
	MemoryManager()
	{
		ULONG HeapInformationValue = 2;

		if (HeapSetInformation(GetProcessHeap(),
			HeapCompatibilityInformation,
			&HeapInformationValue,
			sizeof(HeapInformationValue)) == false)
		{
			assert(false);
			throw std::invalid_argument("heap set information failed");
		}
	}

	~MemoryManager()
	{
		memory_allocators_.clear();
	}

	template<typename T>
	T* allocate(std::size_t size)
	{
		std::shared_ptr<MemoryAllocator> memory_allocator = getMemoryAllocator<T>();
		if (memory_allocator == nullptr)
			return nullptr;

		return static_cast<T*>(memory_allocator->allocate(size));
	}

	template<typename T>
	void deallocate(PVOID ptr, std::size_t size)
	{
		std::shared_ptr<MemoryAllocator> memory_allocator = getMemoryAllocator<T>();
		if (memory_allocator == nullptr)
		{
			/// error
			assert(false);
			return;
		}

		memory_allocator->deallocate(ptr, size);
	}

	template<typename T>
	std::shared_ptr<MemoryAllocator<T>> getMemoryAllocator()
	{
		const std::type_index type_index(typeid(T));
		auto itor = memory_allocators_.find(type_index);
		if (itor == memory_allocators_.end())
		{
			return createMemoryAllocator<T>();
		}

		return std::static_pointer_cast<MemoryAllocator<T>>(itor->second);
	}

private:
	template<typename T>
	std::shared_ptr<MemoryAllocator<T>> createMemoryAllocator()
	{
		const std::type_index type_idx(typeid(T));
		std::shared_ptr<MemoryAllocator<T>> add_allocator = std::make_shared<MemoryAllocator<T>>();

		auto ret = memory_allocators_.emplace(type_idx, add_allocator);
		if (ret.second == false)
			return nullptr;

		return add_allocator;
	}

	MemoryAllocators memory_allocators_;
};

thread_local MemoryManager tg_memory_manager;

MemoryManager& get_thread_local_manager()
{
	return tg_memory_manager;
}

template<typename T>
using AllocTraits = std::allocator_traits<MemoryAllocator<T>>;

template<typename T>
using AllocPtr = typename AllocTraits<T>::template rebind_alloc<T>;

template<typename T>
using AllocPtrTraits = std::allocator_traits<AllocPtr<T>>;

template<typename T>
struct deleter {
	AllocPtr<T> alloc_ptr;
	void operator()(T* ptr) {
		AllocPtrTraits<T>::destroy(alloc_ptr, ptr);
		AllocPtrTraits<T>::deallocate(alloc_ptr, ptr, 1);
	}
};

template<typename T, typename... Args>
//std::unique_ptr<T, typename deleter<T>> allocate_unique(Args&&... args)
std::unique_ptr<T> allocate_unique(Args&&... args)
{
	std::shared_ptr<MemoryAllocator<T>> memory_allocator = get_thread_local_manager().getMemoryAllocator<T>();
	if (memory_allocator == nullptr)
		return nullptr;

	AllocPtr<T> alloc_ptr(*memory_allocator);
	T* ptr = AllocPtrTraits<T>::allocate(alloc_ptr, 1);
	try
	{
		AllocPtrTraits<T>::construct(alloc_ptr, ptr, std::forward<Args>(args)...);
	}
	catch (...)
	{
		AllocPtrTraits<T>::deallocate(alloc_ptr, ptr, 1);
		return nullptr;
	}

	//return std::unique_ptr<T, deleter<T>>(ptr, deleter<T>{ alloc_ptr });
	return std::unique_ptr<T>(ptr);
}

template<typename T, typename... Args>
std::shared_ptr<T> allocate_shared(Args&&... args)
{
	std::shared_ptr<MemoryAllocator<T>> memory_allocator = get_thread_local_manager().getMemoryAllocator<T>();
	if (memory_allocator == nullptr)
		return nullptr;

	AllocPtr<T> alloc_ptr(*memory_allocator);
	T* ptr = AllocPtrTraits<T>::allocate(alloc_ptr, 1);
	try
	{
		AllocPtrTraits<T>::construct(alloc_ptr, ptr, std::forward<Args>(args)...);
	}
	catch (...)
	{
		AllocPtrTraits<T>::deallocate(alloc_ptr, ptr, 1);
		return nullptr;
	}

	return std::shared_ptr<T>(ptr, [alloc_ptr](T* ptr) mutable {
		AllocPtrTraits<T>::destroy(alloc_ptr, ptr);
		AllocPtrTraits<T>::deallocate(alloc_ptr, ptr, 1);
		});
}

template<typename T>
std::vector<T, MemoryAllocator<T>> allocate_vector()
{
	std::shared_ptr<MemoryAllocator<T>> memory_allocator = get_thread_local_manager().getMemoryAllocator<T>();
	if (memory_allocator == nullptr)
	{
#ifdef _DEBUG
		assert(false);
#else
		/// error log
		thread_local MemoryAllocator<T> t_vector_memory_allocator;
		return std::move(std::vector<T, MemoryAllocator<T>>(t_vector_memory_allocator));
#endif
	}

	return std::move(std::vector<T, MemoryAllocator<T>>(*memory_allocator));
}

template<typename T>
inline std::list<T, MemoryAllocator<T>> allocate_list()
{
	std::shared_ptr<MemoryAllocator<T>> memory_allocator = get_thread_local_manager().getMemoryAllocator<T>();
	if (memory_allocator == nullptr)
	{
#ifdef _DEBUG
		assert(false);
#else
		/// error log
		thread_local MemoryAllocator<T> t_list_memory_allocator;
		return std::move(std::list<T, MemoryAllocator<T>>(t_list_memory_allocator));
#endif
	}

	return std::move(std::list<T, MemoryAllocator<T>>(*memory_allocator));
}

template<typename T>
inline std::queue<T, MemoryAllocator<T>> allocate_queue()
{
	std::shared_ptr<MemoryAllocator<T>> memory_allocator = get_thread_local_manager().getMemoryAllocator<T>();
	if (memory_allocator == nullptr)
	{
#ifdef _DEBUG
		assert(false);
#else
		/// error log
		thread_local MemoryAllocator<T> t_queue_memory_allocator;
		return std::move(std::queue<T, MemoryAllocator<T>>(t_queue_memory_allocator));
#endif
	}

	return std::move(std::queue<T, MemoryAllocator<T>>(*memory_allocator));
}

template<typename T>
inline std::deque<T, MemoryAllocator<T>> allocate_deque()
{
	std::shared_ptr<MemoryAllocator<T>> memory_allocator = get_thread_local_manager().getMemoryAllocator<T>();
	if (memory_allocator == nullptr)
	{
#ifdef _DEBUG
		assert(false);
#else
		/// error log
		thread_local MemoryAllocator<T> t_deque_memory_allocator;
		return std::move(std::deque<T, MemoryAllocator<T>>(t_deque_memory_allocator));
#endif
	}

	return std::move(std::deque<T, MemoryAllocator<T>>(*memory_allocator));
}

template<typename T>
inline std::set<T, MemoryAllocator<T>> allocate_set()
{
	std::shared_ptr<MemoryAllocator<T>> memory_allocator = get_thread_local_manager().getMemoryAllocator<T>();
	if (memory_allocator == nullptr)
	{
#ifdef _DEBUG
		assert(false);
#else
		/// error log
		thread_local MemoryAllocator<T> t_set_memory_allocator;
		return std::move(std::set<T, MemoryAllocator<T>>(t_set_memory_allocator));
#endif
	}

	return std::move(std::set<T, MemoryAllocator<T>>(*memory_allocator));
}

template<typename T>
inline std::unordered_set<T, MemoryAllocator<T>> allocate_unordered_set()
{
	std::shared_ptr<MemoryAllocator<T>> memory_allocator = get_thread_local_manager().getMemoryAllocator<T>();
	if (memory_allocator == nullptr)
	{
#ifdef _DEBUG
		assert(false);
#else
		/// error log
		thread_local MemoryAllocator<T> t_unordered_set_memory_allocator;
		return std::move(std::unordered_set<T, MemoryAllocator<T>>(t_unordered_set_memory_allocator));
#endif
	}

	return std::move(std::unordered_set<T, MemoryAllocator<T>>(*memory_allocator));
}

template<typename Key, typename Value>
inline std::map<Key, Value, MemoryAllocator<std::pair<Key, Value>>> allocate_map()
{
	std::shared_ptr<MemoryAllocator<std::pair<Key, Value>>> memory_allocator = get_thread_local_manager().getMemoryAllocator<std::pair<Key, Value>>();
	if (memory_allocator == nullptr)
	{
#ifdef _DEBUG
		assert(false);
#else
		/// error log
		thread_local MemoryAllocator<std::pair<Key, Value>> t_map_memory_allocator;
		return std::move(std::map<Key, Value, MemoryAllocator<std::pair<Key, Value>>>(t_map_memory_allocator));
#endif
	}

	return std::move(std::map<Key, Value, MemoryAllocator<std::pair<Key, Value>>>(*memory_allocator));
}

template<typename Key, typename Value>
inline std::unordered_map<Key, Value, MemoryAllocator<std::pair<Key, Value>>> allocate_unordered_map()
{
	std::shared_ptr<MemoryAllocator<std::pair<Key, Value>>> memory_allocator = get_thread_local_manager().getMemoryAllocator<std::pair<Key, Value>>();
	if (memory_allocator == nullptr)
	{
#ifdef _DEBUG
		assert(false);
#else
		/// error log
		thread_local MemoryAllocator<std::pair<Key, Value>> t_unordered_map_memory_allocator;
		return std::move(std::unordered_map<Key, Value, MemoryAllocator<std::pair<Key, Value>>>(t_unordered_map_memory_allocator));
#endif
	}

	return std::move(std::unordered_map<Key, Value, MemoryAllocator<std::pair<Key, Value>>>(*memory_allocator));
}
#endif


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

struct MemoryPoolInfo
{
	const std::string type_name;
	const std::vector<MemoryChunkInfo> memory_chunk_infos;
};

template<typename T>
class MemoryAllocator
{
public:
	using value_type = T;
	using pointer = T*;
	using const_pointer = const T*;
	using void_pointer = void*;
	using const_void_pointer = const void*;
	using reference = value_type&;
	using const_reference = const value_type;
	using difference_type = std::ptrdiff_t;
	using size_type = std::size_t;

	template<typename U>
	struct rebind {
		using other = MemoryAllocator<U>;
	};

	MemoryAllocator() noexcept
		: type_name_(typeid(T).name()), mem_pool_(std::make_shared<MemoryPool>())
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
		: type_name_(other.type_name_), mem_pool_(other.mem_pool_)
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
		: type_name_(typeid(T).name()), mem_pool_(nullptr)
#ifdef _DEBUG
		, callstack_trace_manager_(nullptr)
#endif
	{
		std::shared_ptr<MemoryAllocator<T>> memory_allocator = get_thread_local_manager().getMemoryAllocator<T>();
		if (memory_allocator == nullptr)
		{
			#ifdef _DEBUG
		assert(false);
#else
		/// error log
		thread_local MemoryAllocator<std::pair<Key, Value>> t_unordered_map_memory_allocator;
		return std::move(std::unordered_map<Key, Value, MemoryAllocator<std::pair<Key, Value>>>(t_unordered_map_memory_allocator));
#endif
		}
		
		mem_pool_ = memory_allocator->mem_pool_;
		assert(mem_pool_ != nullptr);
#ifdef _DEBUG
		callstack_trace_manager_ = memory_allocator->callstack_trace_manager_;
		assert(callstack_trace_manager_ != nullptr);
#endif
	}

	MemoryAllocator<T>& operator=(const MemoryAllocator<T>& other) noexcept
	{
		if (this != &other)
		{
			type_name_ = other.type_name_;
			mem_pool_ = other.mem_pool_;
#ifdef _DEBUG
			callstack_trace_manager_ = other.callstack_trace_manager_;
#endif
		}

		return *this;
	}

	pointer allocate(const size_t size)
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

	template<typename U>
	pointer allocate(const size_t size)
	{
		T* alloc_ptr = mem_pool_->alloc<U>(size);

#ifdef _DEBUG
		DWORD hash;
		CallStack callstack(_default_stack_depth_);
		CaptureStackBackTrace(0, static_cast<DWORD>(_default_stack_depth_), callstack.data(), &hash);

		callstack_trace_manager_->Add<U>(alloc_ptr, callstack);
#endif

		return alloc_ptr;
	}

	void deallocate(pointer const ptr, const size_t size) noexcept 
	{
#ifdef _DEBUG
		callstack_trace_manager_->Remove(ptr);
#endif
		mem_pool_->free<T>(ptr, size);
	}

	template<typename T, typename... Args>
	void construct(pointer ptr, Args&&... args)
	{
		if (ptr == nullptr)
			throw std::bad_alloc();

		new(ptr) T(std::forward<Args>(args)...);
	}

	template<typename T>
	void destroy(pointer ptr) noexcept
	{
		ptr->~T();
	}

	inline pointer address(reference r) { return &r; }
	inline const_pointer address(const_reference r) { return &r; }

	inline bool operator==(MemoryAllocator<T> const&)
	{
		return true;
	}

	inline bool operator!=(MemoryAllocator<T> const& other)
	{
		return !operator==(other);
	}

	MemoryPoolInfo get_memory_pool_info() const
	{
		return MemoryPoolInfo{ type_name_, mem_pool_->get_memory_chunk_infos() };
	}

private:
#ifdef _DEBUG
	std::shared_ptr<CallStackTraceManager> callstack_trace_manager_;
#endif
	const std::string type_name_;
	std::shared_ptr<MemoryPool> mem_pool_;
};