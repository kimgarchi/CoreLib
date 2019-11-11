#pragma once
#include "stdafx.h"
#include "ObjectStation.h"
#include "ObjectManager.h"

template<typename _Ty>
class wrap abstract;

template<typename _Ty>
class wrapper;

template<typename _Ty>
class wrapper_hub;

template<typename _Ty>
class wrapper_node;

//
template<typename _Ty>
class wrap abstract
{
public:
	wrap();
	~wrap();

	_Ty* operator()() { return _data(); }
	_Ty& operator*() { return *_data(); }
	_Ty& operator->() { return *_data(); }

	WORD use_count() { return _use_count(); }

protected:
	inline std::atomic<WORD>& _use_count() { return use_count_; }
	inline void _increase_use_count() { _use_count().fetch_add(1); }
	inline void _decrease_use_count() { _use_count().fetch_sub(1); }4

	virtual _Ty* _data() abstract;
	virtual void _clear() abstract;

private:
	std::atomic<WORD> use_count_;
};

template<typename _Ty>
class wrapper final : public wrap<_Ty>
{
public:
	wrapper(_Ty* data, bool binding = false);
	virtual ~wrapper();

	inline wrapper_hub<_Ty> make_hub() { return wrapper_hub<_Ty>(this); }

private:
	virtual _Ty* _data() override { return data_; }
	virtual void _clear() override;

	_Ty* data_;
	ObjectPool<_Ty>* bind_pool_;
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