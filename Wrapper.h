#pragma once
#include "stdafx.h"

namespace gc
{
	template<typename T>
	class unique_wrapper
	{
	public:
		unique_wrapper(T* data)
			: data_(data), use_count_(0)
		{
			add_use_count();
		}

		unique_wrapper(unique_wrapper<T>& copy_wrapper)
			: data_(copy_wrapper.data_), use_count_(0)
		{
			copy_wrapper.sub_use_count();
			copy_wrapper.data_ = nullptr;
			add_use_count();
		}

		virtual ~unique_wrapper()
		{
			sub_use_count();

			if (use_count() <= 0)
			{
				delete data_;
				data_ = nullptr;
			}
		}

		T* operator()() { return data(); }
		T& operator*() { return *data(); }
		T& operator->() { return *data(); }

		virtual unique_wrapper<T>& operator=(const unique_wrapper<T>&) = delete;

	protected:
		virtual T* data() { return data_; }
		virtual std::atomic<WORD>& use_count() { return use_count_; }

		void add_use_count() { use_count().fetch_add(1); }
		void sub_use_count() { use_count().fetch_sub(1); }

	private:
		T* data_;
		std::atomic<WORD> use_count_;
	};

	template<typename T>
	class shared_wrapper final : public unique_wrapper<T>
	{
	public:
		shared_wrapper(T* template_data)
			: unique_wrapper<T>(template_data), orign_wrapper(nullptr)
		{
		}

		shared_wrapper(shared_wrapper<T>& copy_wrapper)
			: unique_wrapper<T>(nullptr), orign_wrapper(&copy_wrapper)
		{
			unique_wrapper<T>::add_use_count();
		}

		virtual ~shared_wrapper()
		{
			unique_wrapper<T>::sub_use_count();
		}

		virtual std::atomic<WORD>& use_count() override
		{
			if (orign_wrapper != nullptr)
				return orign_wrapper->use_count();

			return unique_wrapper<T>::use_count();
		}

	private:
		virtual T* data() override
		{
			if (orign_wrapper != nullptr)
				return orign_wrapper->data();

			return reinterpret_cast<T*>(unique_wrapper<T>::data());
		}

		shared_wrapper<T>* orign_wrapper;
	};
}