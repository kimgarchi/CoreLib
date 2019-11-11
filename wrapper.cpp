#include "stdafx.h"
#include "wrapper.h"

template<typename _Ty>
wrap<_Ty>::wrap<_Ty>()
	: use_count_(0)
{
}

template<typename _Ty>
wrap<_Ty>::~wrap<_Ty>()
{
	assert(use_count_ != 0);
}

template<typename _Ty>
wrapper<_Ty>::wrapper(_Ty* data, bool binding)
	: data_(data), bind_pool_(nullptr)
{
	assert(data_ != nullptr);

	if (binding)
	{
		// ..binding...
	}
}

template<typename _Ty>
wrapper<_Ty>::~wrapper()
{
	_clear();
}

template<typename _Ty>
void wrapper<_Ty>::_clear()
{
	assert(use_count() == 0);

	if (bind_pool_ == nullptr)
	{
		delete data_;
		data_ = nullptr
	}
	else
	{
		// back...
	}
}

template<typename _Ty>
wrapper_hub<_Ty>::~wrapper_hub()
{
	wrapper_._decrease_use_count();
	if (wrapper_.use_count() == 0)
	{
		if (bind_pool_ == nullptr)
		{

		}
		else
		{

		}
	}
}

template<typename _Ty>
wrapper_hub<_Ty>::wrapper_hub(wrapper<_Ty>& wrapper)
	: wrapper_(wrapper)
{
}

template<typename _Ty>
wrapper_node<_Ty>::~wrapper_node()
{

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