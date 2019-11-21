#pragma once
#include "stdafx.h"
#include "ObjectStation.h"
#include "ObjectManager.h"

namespace
{
	template<typename _Ty>
	class wrap abstract;

	template<typename _Ty>
	class wrapper;

	template<typename _Ty>
	class wrapper_hub;

	template<typename _Ty>
	class wrapper_node;
	
	template<typename _Ty>
	class wrap abstract
	{
	public:
		wrap();
		virtual ~wrap();

		_Ty* operator()() { return _data(); }
		_Ty& operator*() { return *_data(); }
		_Ty& operator->() { return *_data(); }

		inline WORD use_count() { return _use_count(); }

	protected:
		using atomic_count = std::atomic<WORD>;

		inline void _increase_use_count() { _use_count().fetch_add(1); }
		inline void _decrease_use_count() { _use_count().fetch_sub(1); }

		virtual _Ty* _data() abstract;

	private:
		inline atomic_count& _use_count() { return use_count_; }

		atomic_count use_count_;
	};

	template<typename _Ty>
	class wrapper : public wrap<_Ty>
	{
	public:
		explicit wrapper(_Ty* data);
		virtual ~wrapper();

		wrapper_hub<_Ty> make_hub();

	private:
		using BindHub = std::set<wrapper_hub<_Ty>>;

		virtual _Ty* _data() override { return data_; }
		
		_Ty* data_;
		BindHub bind_hub_;
	};
	
	template<typename _Ty>
	class wrapper_hub : public wrap<_Ty>
	{
	public:		
		virtual ~wrapper_hub();
		inline wrapper_node<_Ty> make_node() { return wrapper_node<_Ty>(this); }

	private:
		friend class wrapper<_Ty>;

		wrapper_hub(wrapper<_Ty>& wrapper);
		virtual _Ty* _data() override { return wrapper_(); }

		wrapper<_Ty>& wrapper_;
	};
	/*
	template<typename _Ty>
	class wrapper_node : public wrap<_Ty>
	{
	public:
		virtual ~wrapper_node();

	private:
		friend class wrapper_hub<_Ty>;

		wrapper_node(wrapper_hub<_Ty>& hub);
		wrapper_hub<_Ty>& hub_;

		virtual _Ty* _data() override { return hub_._data(); }
	};
	*/
	template<typename _Ty>
	wrap<_Ty>::wrap()
		: use_count_(0)
	{
	}

	template<typename _Ty>
	wrap<_Ty>::~wrap()
	{
		assert(use_count_ == 0);
	}

	template<typename _Ty>
	wrapper<_Ty>::wrapper(_Ty* data)
		: wrap<_Ty>(), data_(data)
	{
		wrap<_Ty>::_increase_use_count();
	}

	template<typename _Ty>
	wrapper<_Ty>::~wrapper()
	{
	}
		
	template<typename _Ty>
	wrapper_hub<_Ty> wrapper<_Ty>::make_hub()
	{
		return wrapper_hub<_Ty>(this);
	}
	
	template<typename _Ty>
	wrapper_hub<_Ty>::wrapper_hub(wrapper<_Ty>& wrapper)
		: wrapper_(wrapper)
	{
		wrapper_._increase_hub_count();
	}

	template<typename _Ty>
	wrapper_hub<_Ty>::~wrapper_hub()
	{
		wrapper_._decrease_hub_count();
		if (wrapper_.use_count() == 0)
		{

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
