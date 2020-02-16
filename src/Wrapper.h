#pragma once
#include "stdafx.h"
#include "ObjectStation.h"
#include "singleton.h"

#pragma warning (push)
#pragma warning (disable : 4348)
#pragma warning (disable : 4521)

#ifdef _DEBUG
#pragma comment(lib, "dbghelp.lib")

const static size_t default_stack_depth_ = 32;
using Hash = ULONG;
using Stacks = std::vector<void*>;

class CallStack;
class CallStackGroup;
using vPtr = void*;
using vPtrNode = std::map<vPtr, Hash>;
using vPtrs = std::set<vPtr>;
using CallStackByHash = std::map<Hash, CallStack>;
using StackTrace = std::map<TypeID, CallStackGroup>;

class CallStack
{
public:
	CallStack(const Stacks& stacks, vPtr ptr)
		: stacks_(stacks)
	{
		assert(Attach(ptr));
	}

	inline bool Attach(vPtr ptr) { return vptrs_.emplace(ptr).second; }
	inline bool Deattach(vPtr ptr) { return static_cast<bool>(vptrs_.erase(ptr)); }

private:
	const Stacks stacks_;
	vPtrs vptrs_;
};

class CallStackGroup
{
public:
	CallStackGroup(const std::wstring& type_name, const Hash& hash, const Stacks& stacks, vPtr ptr)
		: type_name_(type_name)
	{
		assert(vptr_node_.emplace(ptr, hash).second);
		assert(callstack_by_hash_.emplace(hash, CallStack(stacks, ptr)).second);
	}

	bool Attach(const Hash& hash, const Stacks& stacks, vPtr ptr)
	{
		if (vptr_node_.emplace(ptr, hash).second == false)
		{
			//...
			return false;
		}

		auto itor = callstack_by_hash_.find(hash);
		if (itor == callstack_by_hash_.end())
			return callstack_by_hash_.emplace(hash, CallStack(stacks, ptr)).second;

		return itor->second.Attach(ptr);
	}

	bool Deattach(vPtr ptr)
	{
		if (vptr_node_.find(ptr) == vptr_node_.end())
		{
			//...
			return false;
		}

		const Hash& hash = vptr_node_.at(ptr);
		if (callstack_by_hash_.find(hash) == callstack_by_hash_.end())
		{
			//...
			return false;
		}

		return callstack_by_hash_.at(hash).Deattach(ptr);
	}

private:
	const std::wstring type_name_;
	CallStackByHash callstack_by_hash_;
	vPtrNode vptr_node_;
};

#endif

template<typename _Ty>
class wrapper abstract;

template<typename _Ty>
class wrapper_hub;

template<typename _Ty>
class wrapper_node;

class ObjectProvider : public Singleton<ObjectProvider>
{
public:
	template <typename _Ty, typename ..._Tys, is_object<_Ty> = nullptr>
	_Ty* Lend(_Tys&&... Args)
	{
		if (ObjectStation::GetInstance().IsBinding<_Ty>() == false)
			ObjectStation::GetInstance().BindObjectPool<_Ty>();

		_Ty* object = ObjectStation::GetInstance().Pop<_Ty>(Args...);

#ifdef _DEBUG
		Hash hash;
		Stacks stacks(default_stack_depth_);
		CaptureStackBackTrace(0, static_cast<DWORD>(default_stack_depth_), stacks.data(), &hash);
		
		assert(AttackCallStack<_Ty>(hash, stacks, object));
#endif
		return object;
	}

	template<typename _Ty, is_object<_Ty> = nullptr>
	void Refund(_Ty*& data, TypeID type_id)
	{
#ifdef _DEBUG
		assert(DeattachCallStack(data, type_id));
#endif
		if (ObjectStation::GetInstance().Push<_Ty>(data, type_id) == false)
			assert(false);
	}

#ifdef _DEBUG
private:
	template<typename _Ty>
	bool AttackCallStack(const Hash& hash, const Stacks& stacks, vPtr ptr)
	{
		TypeID tid = typeid(_Ty).hash_code();
		auto name = std::string(typeid(_Ty).name());
		std::wstring tname;
		tname.assign(name.begin(), name.end());

		if (stack_trace_.find(tid) == stack_trace_.end())
			return stack_trace_.emplace(tid, CallStackGroup(tname, hash, stacks, ptr)).second;

		return stack_trace_.at(tid).Attach(hash, stacks, ptr);
	}

	bool DeattachCallStack(const vPtr ptr, TypeID type_id)
	{
		if (stack_trace_.find(type_id) == stack_trace_.end())
			return false;

		return stack_trace_.at(type_id).Deattach(ptr);
	}

	StackTrace stack_trace_;
#endif
};

template <typename _Ty, typename ..._Tys>
_NODISCARD wrapper_hub<_Ty> make_wrapper_hub(_Tys&&... Args)
{
	return wrapper_hub<_Ty>(ObjectProvider::GetInstance().Lend<_Ty, _Tys...>(Args...));
}

template<typename _Ty>
class wrapper abstract
{
public:
	wrapper(_Ty* data, TypeID type_id = typeid(_Ty).hash_code()) 
		: data_(data), type_id_(type_id)
	{
		ASSERT(data_ != nullptr, L"wrapper_hub new failed...");
	}

	virtual ~wrapper() 
	{
		if (_use_count() > 0 || _node_count() > 0)
			return;

		ObjectProvider::GetInstance().Refund<_Ty>(data_, type_id_);
	}

	_Ty* get() { return _data(); }
	_Ty* operator->() { return get(); }
	_Ty& operator*() { return *_data(); }

	const Count& use_count() { return _use_count(); }
	const Count& node_count() { return _node_count(); }

protected:
	inline Count& _use_count() { return get()->use_count(); }
	inline Count& _node_count() { return get()->node_count(); }	

	void _increase_use_count() { _use_count().fetch_add(1); }
	void _decrease_use_count() { _use_count().fetch_sub(1); }

	void _increase_node_count() { _node_count().fetch_add(1); }
	void _decrease_node_count() { _node_count().fetch_sub(1); }

	inline _Ty* _data() { return data_; }

private:
	_Ty* data_;
	TypeID type_id_;
};

template<typename _Ty>
class wrapper_hub final : public wrapper<_Ty>
{
public:
	void operator=(const wrapper_hub<_Ty>&) = delete;
	
	template<typename _rTy>
	wrapper_hub(wrapper_hub<_rTy> hub)
		: wrapper<_Ty>(hub.get(), typeid(_rTy).hash_code())
	{
		wrapper<_Ty>::_increase_use_count();
	}
	
	wrapper_hub(const wrapper_hub<_Ty>& hub)
		: wrapper<_Ty>(const_cast<wrapper_hub<_Ty>&>(hub).get())
	{
		wrapper<_Ty>::_increase_use_count();
	}

	virtual ~wrapper_hub()
	{
		wrapper<_Ty>::_decrease_use_count();
	}

	_NODISCARD wrapper_node<_Ty> make_node() { return wrapper_node<_Ty>(wrapper<_Ty>::get()); }

private:
	template <typename _Ty, typename ..._Tys>
	friend wrapper_hub<_Ty> make_wrapper_hub(_Tys&&...);

	wrapper_hub(_Ty* data)
		: wrapper<_Ty>(data, typeid(_Ty).hash_code())
	{
		wrapper<_Ty>::_increase_use_count();
	}
};

template<typename _Ty>
class wrapper_node : public wrapper<_Ty>
{
public:
	void operator=(const wrapper_hub<_Ty>&) = delete;
	void operator=(const wrapper_node<_Ty>&) = delete;
	
	wrapper_node(wrapper_hub<_Ty>& hub)
		: wrapper<_Ty>(hub.get())
	{
		wrapper<_Ty>::_increase_node_count();
	}

	wrapper_node(const wrapper_node<_Ty>& node)
		: wrapper<_Ty>(const_cast<wrapper_node<_Ty>&>(node).get())
	{
		wrapper<_Ty>::_increase_node_count();
	}

	virtual ~wrapper_node()
	{
		wrapper<_Ty>::_decrease_node_count();
	}

private:
	friend class wrapper_hub<_Ty>;
	wrapper_node(_Ty* data)
		: wrapper<_Ty>(data, typeid(_Ty).hash_code())
	{
		wrapper<_Ty>::_increase_node_count();
	}
};

#pragma warning (pop)