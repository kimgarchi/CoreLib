#pragma once

#include "stdafx.h"
#include "ObjectStation.h"
#include "ObjectManager.h"

#pragma warning (push)
#pragma warning (disable : 4348)

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
inline constexpr wrapper_hub<_Ty, _Size> make_wrapper_hub(_Tys&&... _Args)
{
	return wrapper_hub<_Ty, _Size>(new _Ty(std::forward<_Tys>(_Args)...));
}

template<typename _Ty, typename _Size>
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

	virtual _Ty* _data() abstract;
};

template<typename _Ty, typename _Size>
class wrapper_hub final : public wrapper<_Ty, _Size>
{
public:
	using Hub = wrapper_hub<_Ty, _Size>;

	wrapper_hub(const Hub& hub);
	
	virtual ~wrapper_hub();

	wrapper_node<_Ty, _Size> make_node() { return wrapper_node<_Ty, _Size>(this); }
	virtual std::atomic<_Size>& use_count() override { return hub().use_count_; }

private:
	template <typename _Ty, typename _Size = BYTE, typename... _Tys,
		typename is_numberic<_Size>::type * = nullptr>
		friend constexpr wrapper_hub<_Ty, _Size> make_wrapper_hub(_Tys&&... _Args);

	friend class wrapper_node<_Ty, _Size>;
	
	void _increase_node_count() { _node_count().fetch_add(1); }
	void _decrease_node_count() { _node_count().fetch_sub(1); }

	std::atomic<_Size>& _node_count() { return node_count_; }

	wrapper_hub(_Ty* data = nullptr);
	virtual _Ty* _data() override { return hub().data_; }

	bool is_main_hub() { return hub_ == nullptr ? true : false; }
	Hub& hub() { return is_main_hub() == true ? *this : hub_->hub(); }

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
		ASSERT(false, L"wraping data is nullptr");
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
		{
			if (_node_count() != 0)
				ASSERT(false, L"remain node... invalid wrapper delete");

			SAFE_DELETE(data_);
		}			
	}			
}

// don't put in a container
template<typename _Ty, typename _Size>
class wrapper_node : public wrapper<_Ty, _Size>
{
public:
	virtual ~wrapper_node();
	inline virtual _Ty* _data() override { return bind_hub_._data(); }

private:
	friend class wrapper_hub<_Ty, _Size>;

	wrapper_node(const wrapper_hub<_Ty, _Size>& hub);
	wrapper_hub<_Ty, _Size>& bind_hub_;
};

template<typename _Ty, typename _Size>
wrapper_node<_Ty, _Size>::~wrapper_node()
{
	bind_hub_._decrease_node_count();
}

template<typename _Ty, typename _Size>
wrapper_node<_Ty, _Size>::wrapper_node(const wrapper_hub<_Ty, _Size>& hub)
	: bind_hub_(hub)
{
	bind_hub_._increase_node_count();
}

#pragma warning (pop)