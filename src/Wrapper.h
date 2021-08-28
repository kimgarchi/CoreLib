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
using Stacks = std::vector<PVOID>;
using vPtrNode = std::map<PVOID, Hash>;
using vPtrs = std::set<PVOID>;

class CallStack
{
public:
	CallStack(const Stacks& stacks, PVOID ptr)
		: stacks_(stacks)
	{
		assert(Attach(ptr));
	}

	inline bool Attach(PVOID ptr) { return vptrs_.emplace(ptr).second; }
	inline bool Deattach(PVOID ptr) { return static_cast<bool>(vptrs_.erase(ptr)); }

private:
	const Stacks stacks_;
	vPtrs vptrs_;
};

class CallStackGroup
{
public:
	CallStackGroup(const std::wstring& type_name, const Hash& hash, const Stacks& stacks, PVOID ptr)
		: type_name_(type_name)
	{
		assert(vptr_node_.emplace(ptr, hash).second);
		assert(callstack_by_hash_.emplace(hash, CallStack(stacks, ptr)).second);
	}

	bool Attach(const Hash& hash, const Stacks& stacks, PVOID ptr)
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

	bool Deattach(PVOID ptr)
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
	std::map<Hash, CallStack> callstack_by_hash_;
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

template <typename _Ty, typename ..._Tys>
_NODISCARD wrapper_hub<_Ty> make_wrapper_hub(_Tys&&... Args)
{
	return wrapper_hub<_Ty>(ObjectProvider::GetInstance().Lend<_Ty, _Tys...>(Args...));
}

template<typename _Ty>
class wrapper abstract
{
public:
	wrapper(_Ty* data, TypeID type_id)
		: data_(data), type_id_(type_id)
	{}

	virtual ~wrapper() 
	{
		try_cleanup();
	}

	_Ty* get() { return _data(); }
	_Ty* operator->() { return get(); }
	_Ty& operator*() { return *_data(); }

	const Count& use_count() { return _use_count(); }
	const Count& node_count() { return _node_count(); }

	inline TypeID tid() const { return type_id_; }

	void operator=(const wrapper<_Ty>& wrap)
	{
		_decrease_count();
		try_cleanup();
		object_change(wrap);
		_increase_count();
	}

protected:
	inline Count& _use_count() { return get()->use_count(); }
	inline Count& _node_count() { return get()->node_count(); }	

	virtual void _increase_count() abstract;
	virtual void _decrease_count() abstract;
	
	inline _Ty* _data() { return data_; }

	void try_cleanup()
	{
		if (_use_count() > 0 || _node_count() > 0)
			return;

		ObjectProvider::GetInstance().Refund<_Ty>(data_, type_id_);
	}
	
	void object_change(const wrapper<_Ty>& wrap)
	{
		data_ = wrap.data_;
		type_id_ = wrap.type_id_;
	}

	_Ty* data_;
	TypeID type_id_;
};

template<typename _Ty>
class wrapper_hub final : public wrapper<_Ty>
{
public:
	template<typename _rTy>
	wrapper_hub(wrapper_hub<_rTy> hub)
		: wrapper<_Ty>(hub.get(), hub.tid())
	{
		_increase_count();
	}
	
	wrapper_hub(const wrapper_hub<_Ty>& hub)
		: wrapper<_Ty>(const_cast<wrapper_hub<_Ty>&>(hub).get(), hub.tid())
	{
		_increase_count();
	}

	virtual ~wrapper_hub()
	{
		_decrease_count();
	}

	_NODISCARD wrapper_node<_Ty> make_node() { return wrapper_node<_Ty>(wrapper<_Ty>::get(), this->tid()); }
	
private:
	template <typename _Ty, typename ..._Tys>
	friend wrapper_hub<_Ty> make_wrapper_hub(_Tys&&...);

	wrapper_hub(_Ty* data)
		: wrapper<_Ty>(data, typeid(_Ty).hash_code())
	{
		_increase_count();
	}

	virtual void _increase_count() override { wrapper<_Ty>::_use_count().fetch_add(1); }
	virtual void _decrease_count() override { wrapper<_Ty>::_use_count().fetch_sub(1); }
};

template<typename _Ty>
class wrapper_node : public wrapper<_Ty>
{
public:
	wrapper_node(wrapper_hub<_Ty>& hub)
		: wrapper<_Ty>(hub.get(), hub.tid())
	{
		_increase_count();
	}

	wrapper_node(const wrapper_node<_Ty>& node)
		: wrapper<_Ty>(const_cast<wrapper_node<_Ty>&>(node).get(), node.tid())
	{
		_increase_count();
	}

	virtual ~wrapper_node()
	{
		_decrease_count();
	}

private:
	friend class wrapper_hub<_Ty>;
	wrapper_node(_Ty* data, TypeID type_id)
		: wrapper<_Ty>(data, type_id)
	{
		_increase_count();
	}

	virtual void _increase_count() override { wrapper<_Ty>::_node_count().fetch_add(1); }
	virtual void _decrease_count() override { wrapper<_Ty>::_node_count().fetch_sub(1); }
};

template <typename _Ty>
_NODISCARD bool operator>(const wrapper<_Ty>& left, const wrapper<_Ty>& right)
{
	return const_cast<wrapper<_Ty>&>(left).get()->priority_value() > const_cast<wrapper<_Ty>&>(right).get()->priority_value();
}

template <typename _Ty>
_NODISCARD bool operator>=(const wrapper<_Ty>& left, const wrapper<_Ty>& right)
{
	return const_cast<wrapper<_Ty>&>(left).get()->priority_value() >= const_cast<wrapper<_Ty>&>(right).get()->priority_value();
}

template <typename _Ty>
_NODISCARD bool operator<(const wrapper<_Ty>& left, const wrapper<_Ty>& right)
{
	return const_cast<wrapper<_Ty>&>(left).get()->priority_value() < const_cast<wrapper<_Ty>&>(right).get()->priority_value();
}

template <typename _Ty>
_NODISCARD bool operator<=(const wrapper<_Ty>& left, const wrapper<_Ty>& right)
{
	return const_cast<wrapper<_Ty>&>(left).get()->priority_value() <= const_cast<wrapper<_Ty>&>(right).get()->priority_value();
}

template <typename _Ty>
_NODISCARD bool operator==(const wrapper<_Ty>& left, const wrapper<_Ty>& right)
{
	return const_cast<wrapper<_Ty>&>(left).get() == const_cast<wrapper<_Ty>&>(right).get();
}

#pragma warning (pop)