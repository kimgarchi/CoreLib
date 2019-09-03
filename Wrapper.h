#pragma once
#include "stdafx.h"

// head
namespace gc
{
	template<typename T>
	class unique_wrapper
	{
	public:
		unique_wrapper(T* data);
		virtual ~unique_wrapper();

		T* operator()() { return data(); }
		T& operator*() { return *data(); }
		T& operator->() { return *data(); }

		virtual bool expired() { return (data_ != nullptr && use_count() > 0); }

	protected:
		virtual std::atomic<WORD>& use_count() { return strong_use_count_; }

		void add_use_count() { assert(use_count() < UINT16_MAX); use_count().fetch_add(1); }
		void sub_use_count() { assert(use_count() > 0); use_count().fetch_sub(1); }

		virtual T* data() { return data_; }

		T* data_;

	private:
		std::atomic<WORD> strong_use_count_;
	};

	template<typename T>
	class shared_wrapper : public unique_wrapper<T>
	{
	public:
		shared_wrapper(T* template_data);
		shared_wrapper(shared_wrapper<T>& copy_wrapper);

		virtual ~shared_wrapper();
		virtual std::atomic<WORD>& use_count() override;

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

	template<typename T>
	class weak_wrapper
	{
	public:
		weak_wrapper(shared_wrapper<T>& origin_wrapper);
		virtual ~weak_wrapper();
	private:


	};
}

namespace gc
{
	// unique
	template<typename T>
	unique_wrapper<T>::unique_wrapper(T* data)
		: data_(data), strong_use_count_(0)
	{}

	template<typename T>
	unique_wrapper<T>::~unique_wrapper()
	{
		if (data_ != nullptr && use_count() == 0)
		{
			delete data_;
			data_ = nullptr;
		}
	}

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
}