#pragma once
#include "stdafx.h"
#include "ObjectManager.h"

template<typename T>
class Wrapper final
{
public:
	Wrapper<T>(const ObjectPool& bind_obj_pool);
	~Wrapper();

	Wrapper(const Wrapper& wr_obj) = delete;
	Wrapper& operator=(const Wrapper&) = delete;
	
private:
	friend class Object;

	using Owner = Object *;
	using CallStack = Object *;	
	using BackTracer = std::map<Owner, CallStack>;

	bool Init();

	Object* object_;
	BackTracer back_tracer_;
	const ObjectPool& bind_obj_pool_;
};