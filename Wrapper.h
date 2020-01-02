#pragma once
#include "stdafx.h"
#include "Object.h"
#include "ObjectStation.h"

#pragma warning (push)
#pragma warning (disable : 4348)
#pragma warning (disable : 4521)

template<typename _Ty>
class wrapper abstract;

template<typename _Ty>
class wrapper_hub;

template<typename _Ty>
class wrapper_node;

template <typename _Ty, typename is_object<_Ty>::type * = nullptr>
constexpr wrapper_hub<_Ty> make_wrapper_hub()
{
	if (ObjectStation::GetInstance().IsBinding<_Ty>() == false)
		ObjectStation::GetInstance().BindObjectPool<_Ty>();
	
	return wrapper_hub<_Ty>(ObjectStation::GetInstance().Pop<_Ty>());
}

template<typename _Ty, typename is_object<_Ty>::type * = nullptr>
void Refund(_Ty*& data)
{
	ObjectStation::GetInstance().Push<_Ty>(data);
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
			Refund<_Ty>(data_);
	}

	_Ty* get() { return _data(); }
	_Ty* operator()() { return _data(); }
	_Ty& operator*() { return *_data(); }
	_Ty& operator->() { return *_data(); }	

	const Count& use_count() { return _use_count(); }
	const Count& node_count() { return _node_count(); }

protected:
	inline Count& _use_count() { return data_->use_cnt(); }
	inline Count& _node_count() { return data_->node_cnt(); }	

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

	wrapper_node<_Ty> make_node() { return wrapper_node<_Ty>(*this); }	
	wrapper_node<_Ty> operator=(wrapper_hub<_Ty>& hub) { return hub.make_node(); }

private:
	template <typename _Ty, typename is_object<_Ty>::type * = nullptr>
	friend constexpr wrapper_hub<_Ty> make_wrapper_hub();
	
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
		: wrapper<_Ty>(node->_data())
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