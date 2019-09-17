#pragma once
#include "stdafx.h"

using _Cnt = WORD;
using _pCnt = _Cnt *;

template<typename _Ty>
class unique_wrapper;

template<typename _Ty>
class wrapper_base;

template<typename _Ty>
class shared_wrapper;

template<typename _Ty>
class weak_wrapper;

template<typename _Ty>
using _pWrapperBase = wrapper_base<_Ty>*;
template<typename _Ty>
using _rWrapperBase = wrapper_base<_Ty> &;

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