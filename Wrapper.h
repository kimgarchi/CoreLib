#pragma once
#include "stdafx.h"
#include "ObjectStation.h"
#include "ObjectManager.h"


template<typename _Ty, typename _Size = BYTE>
class wrapper abstract;

template<typename _Ty, typename _Size = BYTE>
class wrapper_hub;

template<typename _Ty, typename _Size = BYTE>
class wrapper_node;

template<typename _Ty, typename _Size = BYTE>
class wrapper abstract
{
public:
	_Ty* operator()() { return _data(); }
	_Ty& operator*() { return *_data(); }
	_Ty& operator->() { return *_data(); }

	virtual std::atomic<_Size>& use_count() abstract;

protected:
	void _increase_use_count() { use_count().fetch_add(1); }
	void _decrease_use_count() { use_count().fetch_sub(1); }

	void _increase_node_count() { _node_count().fetch_add(1); }
	void _decrease_node_count() { _node_count().fetch_sub(1); }

	std::atomic<_Size>& _node_count() { return _node_count; }

	virtual _Ty* _data() abstract;
};	

template<typename _Ty, typename _Size = BYTE>
class wrapper_hub final : public wrapper<_Ty, _Size>
{
public:
	using Hub = wrapper_hub<_Ty, _Size>;

	wrapper_hub(_Ty* data = nullptr);
	wrapper_hub(const Hub& hub);
	
	virtual ~wrapper_hub();

	wrapper_node<_Ty, _Size> make_node() { return wrapper_node<_Ty, _Size>(this); }

	virtual std::atomic<_Size>& use_count() override { return hub().use_count_; }

private:
	virtual _Ty* _data() override { return hub().data_; }
	Hub& hub() { return hub_ == nullptr ? *this : hub_->hub(); }

	Hub* hub_;
	_Ty* data_;
	std::atomic<_Size> use_count_;
	std::atomic<_Size> node_count_;
};
	
template<typename _Ty, typename _Size>
wrapper_hub<_Ty, _Size>::wrapper_hub(_Ty* data)
	: hub_(nullptr), data_(data), use_count_(0), node_count_(0)
{
	if (data_ == nullptr)
	{
		ASSERT(false, L"...");
		// pooling object...
	}
		
	ASSERT(data_ != nullptr, L"wrapper_hub new failed...");		
	wrapper<_Ty, _Size>::_increase_use_count();
}

template<typename _Ty, typename _Size>
wrapper_hub<_Ty, _Size>::wrapper_hub(const Hub& hub)
	: hub_(&hub), data_(nullptr), use_count_(0), node_count_(0)
{
	wrapper<_Ty, _Size>::_increase_use_count();
}

template<typename _Ty, typename _Size>
wrapper_hub<_Ty, _Size>::~wrapper_hub()
{
	wrapper<_Ty, _Size>::_decrease_use_count();
	if (hub_ == nullptr)
	{
		if (use_count_ == 0)
			SAFE_DELETE(data_);
	}			
}

template <typename _Ty, typename... _Tys>
constexpr wrapper_hub<_Ty> make_wrapper_hub(_Tys&&... _Args)
{
	return wrapper_hub<_Ty>(new _Ty(std::forward<_Tys>(_Args)...));
}

template<typename _Ty, typename _Size>
class wrapper_node : public wrapper<_Ty, _Size>
{
public:
	virtual ~wrapper_node();

private:
	friend wrapper_hub<_Ty, _Size>;

	wrapper_node(wrapper_hub<_Ty, _Size>& hub);
};