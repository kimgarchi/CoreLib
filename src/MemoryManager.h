#pragma once
#include "stdafx.h"
#include "MemoryAllocator.h"

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
		{
			assert(false);
			return nullptr;
		}

		return add_allocator;
	}
	
	MemoryAllocators memory_allocators_;
};

thread_local MemoryManager tg_memory_manager;

MemoryManager& get_thread_local_manager()
{
	return tg_memory_manager;
}


template<typename T, typename... Args>
std::shared_ptr<T> allocate_shared(Args&&... args)
{
	using AllocTraits = std::allocator_traits<MemoryAllocator<T>>;
	using AllocPtr = typename AllocTraits::template rebind_alloc<T>;
	using AllocPtrTraits = std::allocator_traits<AllocPtr>;

 	std::shared_ptr<MemoryAllocator<T>> memory_allocator = get_thread_local_manager().getMemoryAllocator<T>();
	if (memory_allocator == nullptr)
		return nullptr;

	AllocPtr alloc_ptr(*memory_allocator);
	T* ptr = AllocPtrTraits::allocate(alloc_ptr, 1);
	try
	{
		AllocPtrTraits::construct(alloc_ptr, ptr, std::forward<Args>(args)...);
	}
	catch (...)
	{
		AllocPtrTraits::deallocate(alloc_ptr, ptr, 1);
		throw;
	}

	return std::shared_ptr<T>(ptr, [alloc_ptr](T* ptr) mutable {
		AllocPtrTraits::destroy(alloc_ptr, ptr);
		AllocPtrTraits::deallocate(alloc_ptr, ptr, 1);
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