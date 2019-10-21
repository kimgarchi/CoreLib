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
private:
	std::atomic<WORD> use_count_;
	ObjectPool<_Ty>* bind_pool_;

protected:
	wrap();
	virtual ~wrap();

	inline std::atomic<WORD>& _use_count() { return use_count_; }
	inline void _increase_use_count() { _use_count().fetch_add(1); }
	inline void _decrease_use_count() { _use_count().fetch_sub(1); }

	virtual _Ty* _data() abstract;

public:
	_Ty* operator()() { return _data(); }
	_Ty& operator*() { return *_data(); }
	_Ty& operator->() { return *_data(); }

	WORD use_count() { return _use_count(); }
};


//
template<typename _Ty>
class wrapper final : public wrap<_Ty>
{
private:
	wrapper(_Ty* data);
	wrapper();

	virtual _Ty* _data() override { return data_; }

	_Ty* data_;
public:
	virtual ~wrapper();
};

//
template<typename _Ty>
class wrapper_hub : public wrap<_Ty>
{
private:
	wrapper_hub(wrapper<_Ty>& wrapper);
	virtual _Ty* _data() override { return wrapper_(); }

	wrapper<_Ty>& wrapper_;

public:
	virtual ~wrapper_hub();
	wrapper_node<_Ty> make_node();
};

template<typename _Ty>
class wrapper_node : public wrap<_Ty>
{
private:
	wrapper_node(wrapper_hub<_Ty>& hub);
	wrapper_hub<_Ty>& hub_;

	virtual _Ty* _data() override { return hub_._data(); }

public:
	virtual ~wrapper_node();
};