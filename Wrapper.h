#pragma once
#include "stdafx.h"
#include "ObjectStation.h"
#include "ObjectManager.h"

namespace
{
	template<typename _Ty, typename _Size>
	class wrapper abstract;

	template<typename _Ty, typename _Size>
	class wrapper_hub;

	template<typename _Ty, typename _Size>
	class wrapper_node;
}

namespace
{
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

		virtual _Ty* _data() abstract;
	};	

	template<typename _Ty, typename _Size = BYTE>
	class wrapper_hub final : public wrapper<_Ty, _Size>
	{
	public:
		using Hub = wrapper_hub<_Ty, _Size>;

		wrapper_hub(_Ty* data = nullptr);
		wrapper_hub(Hub& hub = nullptr);
		virtual ~wrapper_hub();

		wrapper_node<_Ty, _Size> make_node() { return wrapper_node<_Ty, _Size>(this); }

		virtual std::atomic<_Size>& use_count() override { return hub().use_count_; }

	private:
		virtual _Ty* _data() override { return hub().data_; }
		Hub& hub() { return hub_ == nullptr ? *this : hub_->hub(); }

		Hub* hub_;
		_Ty* data_;
		std::atomic<_Size> use_count_;
	};
	
	template<typename _Ty, typename _Size>
	wrapper_hub<_Ty, _Size>::wrapper_hub(_Ty* data)
		: hub_(nullptr), data_(data), use_count_(0)
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
	wrapper_hub<_Ty, _Size>::wrapper_hub(Hub& hub)
		: hub_(&hub), data_(nullptr), use_count_(0)
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
}

/*
template<typename _Ty>
class unique_wrapper final
{
public:
	unique_wrapper(_Ty* data);
	~unique_wrapper();

	_Ty* operator()() { return data(); }
	_Ty& operator*() { return *data(); }
	_Ty& operator->() { return *data(); }

private:
	_Ty* data() { return data_; }

	_Ty* pdata_;
};

template<typename _Ty>
unique_wrapper<_Ty>::unique_wrapper(_Ty* data)
	: data_(data)
{}

template<typename T>
unique_wrapper<T>::~unique_wrapper()
{
	if (data_ != nullptr)
	{
		delete data_;
		data_ = nullptr;
	}
}

template<typename _Ty>
class wrapper_base abstract
{
private:
	_Ty* wrapping_data_;
	_pCnt use_count_;

public:
	wrapper_base(_Ty* data)
		: wrapping_data_(data)
	{
		use_count_ = new _Cnt(0);
	}

	wrapper_base(_Ty* data, _pCnt use_count)
		: wrapping_data_(data), use_count_(use_count)
	{
	}

	virtual ~wrapper_base() {}

	_Ty* operator()() { return wrapping_data(); }
	_Ty& operator*() { return *wrapping_data(); }
	_Ty& operator->() { return *wrapping_data(); }

	virtual WORD use_count() { return *ref_use_count(); }

protected:
	virtual _Ty* wrapping_data() { return wrapping_data_; }
	_pCnt ref_use_count() { return use_count_; }

	void add_use_count() { *ref_use_count() += 1; }
	void sub_use_count() { *ref_use_count() -= 1; }
};

template<typename _Ty>
class shared_wrapper final : public wrapper_base<_Ty>
{
public:
	shared_wrapper(_Ty* data)
		: wrapper_base<_Ty>(data)
	{
		this->add_use_count();
	}

	shared_wrapper(shared_wrapper<_Ty>& origin_wrapper)
		: wrapper_base<_Ty>(origin_wrapper.wrapping_data(), origin_wrapper.ref_use_count())
	{
		this->add_use_count();
	}

	virtual ~shared_wrapper()
	{
		this->sub_use_count();
	}

	void operator=(unique_wrapper<_Ty>&) = delete;
	void operator=(shared_wrapper<_Ty>&) = delete;
};
*/
