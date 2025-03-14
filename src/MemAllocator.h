#pragma once

#include "MemoryManager.h"

class MemAllocator 
{
public:
	MemAllocator() = default;

	template<typename _Ty>
	constexpr MemAllocator(const MemAllocator&) noexcept {}

	template<typename _Ty>
	_Ty* allocate(std::size_t n) 
	{
		if (n > std::allocator_traits<MemAllocator>::max_size(*this)) {
			throw std::bad_alloc();
		}

		mem_manager_.Pop();

		return static_cast<T*>(::operator new(n * sizeof(T)));
	}

	template<typename _Ty>
	void deallocate(_Ty* p, std::size_t) noexcept {
		::operator delete(p);
	}

	template<typename _Ty, typename... Args>
	void construct(_Ty* ptr, Args&&... args) {
		new(ptr) _Ty(std::forward<Args>(args)...);
	}

	template<typename _Ty>
	void destroy(_Ty* p) noexcept {
		p->~_Ty();
	}

private:
	MemoryManager mem_manager_;
};

/*
class ObjectProvider : public Singleton<ObjectProvider>
{
public:
	template <typename _Ty, typename ..._Tys, is_object_base<_Ty> = nullptr>
	_Ty* Lend(_Tys&&... Args)
	{
		if (ObjectStation::GetInstance().IsBinding<_Ty>() == false)
			ObjectStation::GetInstance().BindObjectPool<_Ty>();

		_Ty* object = ObjectStation::GetInstance().Pop<_Ty>(Args...);

#ifdef _DEBUG
		Hash hash;
		Stacks stacks(default_stack_depth_);
		CaptureStackBackTrace(0, static_cast<DWORD>(default_stack_depth_), stacks.data(), &hash);

		assert(AttachCallStack<_Ty>(hash, stacks, object));
#endif
		return object;
	}

	template<typename _Ty, is_object_base<_Ty> = nullptr>
	void Refund(_Ty*& data, TypeID type_id)
	{
#ifdef _DEBUG
		assert(DeattachCallStack(static_cast<PVOID>(data), type_id));
#endif
		if (ObjectStation::GetInstance().Push<_Ty>(data, type_id) == false)
			assert(false);
	}

#ifdef _DEBUG
private:
	template<typename _Ty>
	bool AttachCallStack(const Hash& hash, const Stacks& stacks, PVOID ptr)
	{
		TypeID tid = typeid(_Ty).hash_code();
		auto name = std::string(typeid(_Ty).name());
		std::wstring tname;
		tname.assign(name.begin(), name.end());

		if (stack_trace_.find(tid) == stack_trace_.end())
			return stack_trace_.emplace(tid, CallStackGroup(tname, hash, stacks, ptr)).second;

		return stack_trace_.at(tid).Attach(hash, stacks, ptr);
	}

	bool DeattachCallStack(const PVOID ptr, TypeID type_id)
	{
		if (stack_trace_.find(type_id) == stack_trace_.end())
			return false;

		return stack_trace_.at(type_id).Deattach(ptr);
	}

	std::map<TypeID, CallStackGroup> stack_trace_;
#endif
};

*/