#pragma once
#include "stdafx.h"
#include "ObjectStation.h"
#include "singleton.h"

#pragma warning (push)
#pragma warning (disable : 4348)
#pragma warning (disable : 4521)

template<typename _Ty>
class wrapper abstract;

template<typename _Ty>
class wrapper_hub;

template<typename _Ty>
class wrapper_node;

class Packer : public Singleton<Packer>
{
public:
	template <typename _Ty, typename ..._Tys, is_object<_Ty> = nullptr>
	constexpr decltype(auto) CreateHub(_Tys&&... Args)
	{
		if (ObjectStation::GetInstance().IsBinding<_Ty>() == false)
			ObjectStation::GetInstance().BindObjectPool<_Ty>();

		return wrapper_hub<_Ty>(ObjectStation::GetInstance().Pop<_Ty>(Args...));
	}

	template<typename _Ty, is_object<_Ty> = nullptr>
	void Refund(_Ty*& data)
	{
		if (ObjectStation::GetInstance().Push<_Ty>(data) == false)
			assert(false);
	}
};

template <typename _Ty, typename ..._Tys>
constexpr decltype(auto) make_wrapper_hub(_Tys&&... Args)
{
	return Packer::GetInstance().CreateHub<_Ty, _Tys...>(Args...);
}

template<typename _Ty>
class wrapper abstract
{
public:
	wrapper(_Ty* data) 
		: data_(data) 
	{
		ASSERT(data_ != nullptr, L"wrapper_hub new failed...");
	}

	virtual ~wrapper() 
	{
		if (_use_count() == 0 && _node_count() == 0)
			Packer::GetInstance().Refund<_Ty>(data_);
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
};

template<typename _Ty>
class wrapper_hub final : public wrapper<_Ty>
{
public:
	wrapper_hub(const wrapper_hub<_Ty>& hub)
		: wrapper<_Ty>(const_cast<wrapper_hub<_Ty>&>(hub).get())
	{
		wrapper<_Ty>::_increase_use_count();
	}

	wrapper_hub(wrapper_hub<_Ty>& hub)
		: wrapper<_Ty>(hub.get())
	{
		wrapper<_Ty>::_increase_use_count();
	}

	virtual ~wrapper_hub()
	{
		wrapper<_Ty>::_decrease_use_count();
	}

	decltype(auto) make_node() { return wrapper_node<_Ty>(*this); }
	decltype(auto) operator=(wrapper_hub<_Ty>& hub) { return hub.make_node(); }

private:
	friend class Packer;
	
	friend class wrapper_node<_Ty>;

	wrapper_hub(_Ty* data)
		: wrapper<_Ty>(data)
	{
		wrapper<_Ty>::_increase_use_count();
	}
};

template<typename _Ty>
class wrapper_node : public wrapper<_Ty>
{
public:	
	wrapper_node(wrapper_node<_Ty>& node) = delete;
	wrapper_node<_Ty>& operator=(const wrapper_node<_Ty>&) = delete;

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

	wrapper_node(const wrapper_hub<_Ty>& hub)
		: wrapper<_Ty>(const_cast<wrapper_hub<_Ty>&>(hub).get())
	{
		wrapper<_Ty>::_increase_node_count();
	}
};

#pragma warning (pop)