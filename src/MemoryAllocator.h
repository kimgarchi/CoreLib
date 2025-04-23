#pragma once
#include "stdafx.h"
#include "MemoryPool.h"
#include "LockObject.h"

#ifndef MEMORY_MANAGER_H_INCLUDED
#define MEMORY_MANAGER_H_INCLUDED

template<typename T>
class MemoryAllocator;

class MemoryManager
{
private:
	using MemoryPools = std::unordered_map<std::type_index, std::shared_ptr<VOID>>;

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

	MemoryManager(const MemoryManager&) = delete;
	MemoryManager(const MemoryManager&&) = delete;
	void operator=(const MemoryManager&) = delete;
	void operator=(const MemoryManager&&) = delete;

	~MemoryManager()
	{
		memory_pools_.clear();
	}

	template<typename T>
	std::shared_ptr<MemoryPool<T>> getMemoryPool()
	{
		SingleLock sl(sync_mutex_);

		const std::type_index type_index(typeid(T));
		auto itor = memory_pools_.find(type_index);
		if (itor == memory_pools_.end())
		{
			return createMemoryPool<T>();
		}

		return std::static_pointer_cast<MemoryPool<T>>(itor->second);
	}

private:
	template<typename T>
	std::shared_ptr<MemoryPool<T>> createMemoryPool()
	{
		const std::type_index type_idx(typeid(T));
		std::shared_ptr<MemoryPool<T>> add_memory_pool = std::make_shared<MemoryPool<T>>();

		auto ret = memory_pools_.emplace(type_idx, add_memory_pool);
		if (ret.second == false)
			return nullptr;

		return add_memory_pool;
	}

	SyncMutex sync_mutex_;
	MemoryPools memory_pools_;
};

[[nodiscard("get memory manager ref var abandon")]] inline MemoryManager& get_thread_local_manager()
{
	thread_local MemoryManager tg_memory_manager;
	return tg_memory_manager;
}

///////////////////////////////////////////////

template<typename T>
using AllocTraits = std::allocator_traits<MemoryAllocator<T>>;

template<typename T>
using RebindAlloc = typename AllocTraits<T>::template rebind_alloc<T>;

template<typename T>
using RebindAllocTraits = std::allocator_traits<RebindAlloc<T>>;

template<typename T>
struct deleter {
	RebindAlloc<T> alloc_ptr;
	void operator()(T* ptr) {
		RebindAllocTraits<T>::destroy(alloc_ptr, ptr);
		RebindAllocTraits<T>::deallocate(alloc_ptr, ptr, 1);
	}
};

template<typename T, typename... Args>
std::unique_ptr<T, typename deleter<T>> allocate_unique(Args&&... args)
{
	MemoryAllocator<T> memory_allocator(get_thread_local_manager());

	RebindAlloc<T> rebind_alloc(memory_allocator);
	T* alloc_ptr = RebindAllocTraits<T>::allocate(rebind_alloc, 1);
	try
	{
		RebindAllocTraits<T>::construct(rebind_alloc, alloc_ptr, std::forward<Args>(args)...);
	}
	catch (...)
	{
		RebindAllocTraits<T>::deallocate(rebind_alloc, alloc_ptr, 1);
		return nullptr;
	}

	return std::unique_ptr<T, deleter<T>>(alloc_ptr, deleter<T>{ rebind_alloc });
}

template<typename T, typename... Args>
std::shared_ptr<T> allocate_shared(Args&&... args)
{
	MemoryAllocator<T> memory_allocator(get_thread_local_manager());

	RebindAlloc<T> rebind_alloc(memory_allocator);
	T* alloc_ptr = RebindAllocTraits<T>::allocate(rebind_alloc, 1);
	try
	{
		RebindAllocTraits<T>::construct(rebind_alloc, alloc_ptr, std::forward<Args>(args)...);
	}
	catch (...)
	{
		RebindAllocTraits<T>::deallocate(rebind_alloc, alloc_ptr, 1);
		return nullptr;
	}

	return std::shared_ptr<T>(alloc_ptr, [rebind_alloc](T* alloc_ptr) mutable {
		RebindAllocTraits<T>::destroy(rebind_alloc, alloc_ptr);
		RebindAllocTraits<T>::deallocate(rebind_alloc, alloc_ptr, 1);
		});
}

///////////////////////////////////////////////

template<typename T>
using m_vector = std::vector<T, MemoryAllocator<T>>;

template<typename T>
m_vector<T> allocate_vector()
{
	return m_vector<T>(get_thread_local_manager());
}

template<typename T>
using m_list = std::list<T, MemoryAllocator<T>>;

template<typename T>
inline m_list<T> allocate_list()
{
	return m_list<T>(get_thread_local_manager());
}

template<typename T>
using m_queue = std::queue<T, MemoryAllocator<T>>;

template<typename T>
inline m_queue<T> allocate_queue()
{
	return m_queue<T>(get_thread_local_manager());
}

template<typename T>
using m_deque = std::deque<T, MemoryAllocator<T>>;

template<typename T>
inline m_deque<T> allocate_deque()
{
	return m_deque<T>(get_thread_local_manager());
}

///////////////////////////////////////////////

template<typename T, typename Compare = std::less<T>>
using m_set = std::set<T, Compare, MemoryAllocator<T>>;

template<typename T, typename Compare = std::less<T>>
inline m_set<T, Compare> allocate_set()
{
	return m_set<T, Compare>(get_thread_local_manager());
}

template<typename T>
using m_unordered_set = std::unordered_set<T, std::hash<T>, std::equal_to<T>, MemoryAllocator<T>>;

template<typename T>
inline m_unordered_set<T> allocate_unordered_set()
{
	return m_unordered_set<T>(get_thread_local_manager());
}

template<typename Key, typename Value, typename Compare = std::less<Key>>
using m_map = std::map<
	Key, Value, Compare, MemoryAllocator<std::pair<const Key, Value>>>;

template<typename Key, typename Value, typename Compare = std::less<Key>>
inline m_map<Key, Value, Compare> allocate_map()
{
	return m_map<Key, Value, Compare>(get_thread_local_manager());
}

template<typename Key, typename Value>
using m_unordered_map = std::unordered_map<
	Key, Value, std::hash<Key>, std::equal_to<Key>, MemoryAllocator<std::pair<const Key, Value>>>;

template<typename Key, typename Value>
inline m_unordered_map<Key, Value> allocate_unordered_map()
{
	return m_unordered_map<Key, Value>(get_thread_local_manager());
}

///////////////////////////////////////////////

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

	MemoryAllocator(MemoryManager& memory_manager) noexcept
		: ref_memory_manager_(memory_manager)
	{
	}
	MemoryAllocator<T>(const MemoryAllocator<T>& other) noexcept
		: ref_memory_manager_(other.ref_memory_manager_)
	{
	}
	
	template<typename U>
	friend class MemoryAllocator;

	template<typename U>
	MemoryAllocator(const MemoryAllocator<U>& other) noexcept
		: ref_memory_manager_(other.ref_memory_manager_)
	{
	}

	MemoryAllocator<T>& operator=(const MemoryAllocator<T>& other) noexcept
	{	
		if (this != &other)
		{
			ref_memory_manager_ = other.ref_memory_manager_;
		}

		return *this;
	}

	pointer allocate(const size_t size)
	{
		std::shared_ptr<MemoryPool<T>> memory_pool = ref_memory_manager_.getMemoryPool<T>();
		if (memory_pool == nullptr)
			return nullptr;

		return memory_pool->alloc(size);
	}

	void deallocate(pointer const ptr, const size_t size) noexcept 
	{
		std::shared_ptr<MemoryPool<T>> memory_pool = ref_memory_manager_.getMemoryPool<T>();
		if (memory_pool == nullptr)
		{
			assert(false);
			return;
		}

		memory_pool->free(ptr, size);
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

private:
	MemoryManager& ref_memory_manager_;
};

#endif