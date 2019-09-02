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
		unique_wrapper(unique_wrapper<T>& copy_wrapper) = delete;

		virtual ~unique_wrapper();

		T* operator()() { return data(); }
		T& operator*() { return *data(); }
		T& operator->() { return *data(); }

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
	class shared_wrapper final : public unique_wrapper<T>
	{
	public:
		shared_wrapper(T* template_data);
		shared_wrapper(shared_wrapper<T>& copy_wrapper);
		shared_wrapper(unique_wrapper<T>& copy_wrapper);

		virtual ~shared_wrapper();
		virtual std::atomic<WORD>& use_count() override;

		virtual void operator=(unique_wrapper<T>& orgin_wrapper);

	private:
		virtual T* data() override;

		shared_wrapper<T>* orign_wrapper;
		std::atomic<WORD> weak_use_count_;
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
		: unique_wrapper<T>(template_data), orign_wrapper(nullptr), weak_use_count_(0)
	{
		unique_wrapper<T>::add_use_count();
	}

	template<typename T>
	shared_wrapper<T>::shared_wrapper(shared_wrapper<T>& copy_wrapper)
		: unique_wrapper<T>(nullptr), orign_wrapper(&copy_wrapper), weak_use_count_(0)
	{
		unique_wrapper<T>::add_use_count();
	}

	template<typename T>
	shared_wrapper<T>::shared_wrapper(unique_wrapper<T>& copy_wrapper)
		: unique_wrapper<T>(copy_wrapper.data_), orign_wrapper(nullptr), weak_use_count_(0)
	{
		copy_wrapper.data_ = nullptr;
	}

	template<typename T>
	shared_wrapper<T>::~shared_wrapper()
	{
		unique_wrapper<T>::sub_use_count();
	}

	template<typename T>
	std::atomic<WORD>& shared_wrapper<T>::use_count()
	{
		if (orign_wrapper != nullptr)
			return orign_wrapper->use_count();

		return unique_wrapper<T>::use_count();
	}

	template<typename T>
	void shared_wrapper<T>::operator=(unique_wrapper<T>& orgin_wrapper)
	{
		this->data = orign_wrapper.data_;
		orign_wrapper.data_ = nullptr;
	}

	template<typename T>
	T* shared_wrapper<T>::data()
	{
		if (orign_wrapper != nullptr)
			return orign_wrapper->data();

		return reinterpret_cast<T*>(unique_wrapper<T>::data());
	}
}