#pragma once
#include "stdafx.h"
/*
struct wrapper_count
{
	WORD relation_count;
};

template<typename _Ty, typename _pCnt = wrapper_count * >
class wrapper_base abstract
{
public:
	_Ty* operator()() { return data(); }
	_Ty& operator*() { return *data(); }
	_Ty& operator->() { return *data(); }

protected:
	using _pWrapperBase = wrapper_base *;
	using _rWrapperBase = wrapper_base &;

	virtual _Ty* data() = 0;

	wrapper_base(_pWrapperBase ref_base, _pCnt use_count)
		: ref_base_(ref_base), use_count_(use_count)
	{
	}

	virtual ~wrapper_base() {}

	virtual WORD& use_count()
	{
	}

	_pWrapperBase ref_base_;
	_pCnt use_count_;
};

template<typename _Ty, typename _pCnt = wrapper_count *>
class shared_wrapper : public wrapper_base<_Ty>
{
public:
	shared_wrapper(_Ty* data)
		: wrapper_base<_Ty>(nullptr), data_(data)
	{
		add_use_count();
	}

	shared_wrapper(wrapper_base<_Ty>::_pWrapperBase ref_base)
		: wrapper_base<_Ty>(ref_base), data_(nullptr)
	{
		add_use_count();
	}

	virtual ~shared_wrapper()
	{
		sub_use_count();

		// yet not used ref cnt ...
		if (expire())
		{
			delete data_;
			data_ = nullptr;
		}
	}

	virtual _Ty* data() { return data_; }

	std::atomic<WORD>& use_count()
	{
		if (this->ref_base_ != nullptr)
			return this->ref_base_->use_count();

		return use_count_;
	}

	bool expire()
	{
		if (use_count() == 0)
			return true;

		return false;
	}

private:
	void add_use_count() { use_count().fetch_add(1); }
	void sub_use_count() { use_count().fetch_sub(1); }

	_Ty* data_;
};
*/


// head
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

// unique
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
public:
	_Ty* operator()() { return data(); }
	_Ty& operator*() { return *data(); }
	_Ty& operator->() { return *data(); }

protected:
	using _pWrapperBase = wrapper_base *;
	using _rWrapperBase = wrapper_base &;

	wrapper_base(_Ty* data);
	wrapper_base(_rWrapperBase base);

	virtual ~wrapper_base();

private:
	_Ty* data();

	_Ty* data_;
	_pWrapperBase base_;
};

template<typename _Ty>
wrapper_base<_Ty>::wrapper_base(_Ty* data)
	: data_(data), base_(nullptr)
{
}

template<typename _Ty>
wrapper_base<_Ty>::wrapper_base(_rWrapperBase ref_base)
	: data_(nullptr), base_(base)
{
}

template<typename _Ty>
wrapper_base<_Ty>::~wrapper_base()
{
}

template<typename _Ty>
_Ty* wrapper_base<_Ty>::data()
{
	if (base_ == nullptr)
		return data_;

	return base_->data();
}

template<typename _Ty>
class shared_wrapper : public wrapper_base<_Ty>
{
public:
	shared_wrapper(_Ty* template_data);
	shared_wrapper(shared_wrapper<T>& copy_wrapper);

	virtual ~shared_wrapper();
	std::atomic<WORD>& use_count();

	virtual void operator=(shared_wrapper<T>&) = delete;
	virtual void operator=(unique_wrapper<T>& orgin_wrapper) = delete;

	virtual bool expired() override;

protected:		
	std::atomic<WORD>& weak_use_count();		
	void add_weak_use_count() { weak_use_count_.fetch_add(1); }
	void add_weak_use_count() { weak_use_count_.fetch_sub(1); }

private:
	virtual T* data() override;

	std::atomic<WORD> weak_use_count_;
	shared_wrapper<T>* orign_wrapper_;
};	

// shared
template<typename T>
shared_wrapper<T>::shared_wrapper(T* template_data)
	: unique_wrapper<T>(template_data), orign_wrapper_(nullptr)
{
	this->add_use_count();
}

template<typename T>
shared_wrapper<T>::shared_wrapper(shared_wrapper<T>& copy_wrapper)
	: unique_wrapper<T>(nullptr), orign_wrapper_(&copy_wrapper)
{
	this->add_use_count();
}

template<typename T>
shared_wrapper<T>::~shared_wrapper()
{
	this->sub_use_count();
}

template<typename T>
std::atomic<WORD>& shared_wrapper<T>::use_count()
{
	if (orign_wrapper_ != nullptr)
		return orign_wrapper_->use_count();

	return unique_wrapper<T>::use_count();
}

template<typename T>
inline bool shared_wrapper<T>::expired()
{
	if (weak_use_count() == 0 && use_count() == 0)
		return true;

	return false;
}

template<typename T>
std::atomic<WORD>& shared_wrapper<T>::weak_use_count()
{	
	if (orign_wrapper_ != nullptr)
		return orign_wrapper_->weak_use_count();

	return weak_use_count_;		
}

template<typename T>
T* shared_wrapper<T>::data()
{
	if (orign_wrapper_ != nullptr)
		return orign_wrapper_->data();

	return reinterpret_cast<T*>(unique_wrapper<T>::data());
}

