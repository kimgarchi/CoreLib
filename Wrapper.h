#pragma once
#include "stdafx.h"
#include "Object.h"
#include "ObjectStation.h"

#pragma warning (push)
#pragma warning (disable : 4348)
#pragma warning (disable : 4521)

static ObjectStation _object_station;

template<typename _Ty, typename _Size = BYTE>
class wrapper abstract;

template<typename _Ty, typename _Size = BYTE>
class wrapper_hub;

template<typename _Ty, typename _Size = BYTE>
class wrapper_node;

template<typename _Ty>
using is_numberic = std::enable_if<std::is_integral<_Ty>::value, _Ty>;

template <typename _Ty, typename _Size = BYTE, typename... _Tys,
	typename is_numberic<_Size>::type * = nullptr>
	constexpr wrapper_hub<_Ty, _Size> make_wrapper_hub(_Tys&&... _Args)
{
	return wrapper_hub<_Ty, _Size>(new _Ty(std::forward<_Tys>(_Args)...));
}

template <typename _Ty, typename _Size = BYTE,
	typename is_object<_Ty>::type * = nullptr,
	typename is_numberic<_Size>::type * = nullptr>
constexpr wrapper_hub<_Ty, _Size> make_wrapper_hub()
{
	if (_object_station.IsBinding<_Ty>() == false)
		_object_station.BindObjectPool<_Ty>();
	
	return wrapper_hub<_Ty, _Size>(_object_station.Pop<_Ty>());
}

template<typename _Ty, typename _Size>
class wrapper abstract
{
public:
	_Ty* operator()() { return _data(); }
	_Ty& operator*() { return *_data(); }
	_Ty& operator->() { return *_data(); }

	virtual _Size use_count() abstract;
	
protected:

	virtual std::atomic<_Size>& _use_count() abstract;

	void _increase_use_count() { _use_count().fetch_add(1); }
	void _decrease_use_count() { _use_count().fetch_sub(1); }

	virtual _Ty* _data() abstract;
};

template<typename _Ty, typename _Size>
class wrapper_hub final : public wrapper<_Ty, _Size>
{
public:
	wrapper_hub(const wrapper_hub<_Ty, _Size>& hub);
	wrapper_hub(wrapper_hub<_Ty, _Size>& hub);
	virtual ~wrapper_hub();

	wrapper_node<_Ty, _Size> make_node() { return wrapper_node<_Ty, _Size>(*this); }
	virtual _Size use_count() override { return _use_count(); }
	_Size hub_count() { return use_count() - node_count(); }
	_Size node_count() { return _node_count(); }

	wrapper_node<_Ty, _Size> operator=(wrapper_hub<_Ty, _Size>& hub) { return hub.make_node(); }

private:
	template <typename _Ty, typename _Size = BYTE,
		typename is_object<_Ty>::type * = nullptr,
		typename is_numberic<_Size>::type * = nullptr>
	friend constexpr wrapper_hub<_Ty, _Size> make_wrapper_hub();
	
	template <typename _Ty, typename _Size = BYTE, typename... _Tys,
		typename is_numberic<_Size>::type * = nullptr>
	friend constexpr wrapper_hub<_Ty, _Size> make_wrapper_hub(_Tys&&... _Args);
	
	friend class wrapper_node<_Ty, _Size>;

	std::atomic<_Size>& _node_count() { return hub().node_count_; }
	void _increase_node_count() { _node_count().fetch_add(1); }
	void _decrease_node_count() { _node_count().fetch_sub(1); }

	wrapper_hub(_Ty* data = nullptr);
	virtual _Ty* _data() override { return hub().data_; }

	virtual std::atomic<_Size>& _use_count() override { return hub().use_count_; }
	bool is_main_hub() { return hub_ == nullptr ? true : false; }
	wrapper_hub<_Ty, _Size>& hub() { return is_main_hub() == true ? *this : hub_->hub(); }

	wrapper_hub<_Ty, _Size>* hub_;
	_Ty* data_;
	std::atomic<_Size> use_count_;
	std::atomic<_Size> node_count_;
};
	
template<typename _Ty, typename _Size>
wrapper_hub<_Ty, _Size>::wrapper_hub(const wrapper_hub<_Ty, _Size>& hub)
	: hub_(const_cast<wrapper_hub<_Ty, _Size>*>(&hub)), data_(nullptr), use_count_(0), node_count_(0)
{
	wrapper<_Ty, _Size>::_increase_use_count();
}

template<typename _Ty, typename _Size>
wrapper_hub<_Ty, _Size>::wrapper_hub(wrapper_hub<_Ty, _Size>& hub)
	: hub_(&hub), data_(nullptr), use_count_(0), node_count_(0)
{
	wrapper<_Ty, _Size>::_increase_use_count();
}

template<typename _Ty, typename _Size>
wrapper_hub<_Ty, _Size>::wrapper_hub(_Ty* data)
	: hub_(nullptr), data_(data), use_count_(0), node_count_(0)
{
	ASSERT(data_ != nullptr, L"wrapper_hub new failed...");		
	wrapper<_Ty, _Size>::_increase_use_count();
}

template<typename _Ty, typename _Size>
wrapper_hub<_Ty, _Size>::~wrapper_hub()
{
	wrapper<_Ty, _Size>::_decrease_use_count();
	if (hub_ == nullptr)
	{
		if (use_count_ != 0)
			return;

		// case is object Push...
		// case is not object delete...
	}
}

// don't put in a container
template<typename _Ty, typename _Size>
class wrapper_node : public wrapper<_Ty, _Size>
{
public:	
	wrapper_node(wrapper_node<_Ty, _Size>& node) = delete;
	wrapper_node<_Ty, _Size>& operator=(const wrapper_node<_Ty, _Size>&) = delete;

	wrapper_node(const wrapper_node<_Ty, _Size>& node);

	virtual ~wrapper_node();

private:
	friend class wrapper_hub<_Ty, _Size>;

	wrapper_node(wrapper_hub<_Ty, _Size>& hub);
		
	virtual _Size use_count() override { return _use_count(); }
	virtual std::atomic<_Size>& _use_count() override { return _bind_hub()._use_count(); }
	
	virtual _Ty* _data() override { return _bind_hub()._data(); }
	inline wrapper_hub<_Ty, _Size>& _bind_hub() { return bind_hub_; }
	
	wrapper_hub<_Ty, _Size> bind_hub_;
};

template<typename _Ty, typename _Size>
wrapper_node<_Ty, _Size>::wrapper_node(const wrapper_node<_Ty, _Size>& node)
	: bind_hub_(node.bind_hub_)
{
	_bind_hub()._increase_node_count();
}

template<typename _Ty, typename _Size>
wrapper_node<_Ty, _Size>::~wrapper_node()
{
	_bind_hub()._decrease_node_count();
}

template<typename _Ty, typename _Size>
wrapper_node<_Ty, _Size>::wrapper_node(wrapper_hub<_Ty, _Size>& hub)
	: bind_hub_(hub)
{
	_bind_hub()._increase_node_count();
}

#pragma warning (pop)