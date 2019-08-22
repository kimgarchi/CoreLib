#pragma once
#include "stdafx.h"

class Object;
class ObjectPool;
template<class T>
class Wrapper final
{
public:
	Wrapper(const ObjectPool& bind_obj_pool);
	~Wrapper();

	Wrapper(Wrapper* wr_obj) = delete;
	


private:
	friend class Object;

	using Owner = Object *;
	using CallStack = Object *;	
	using BackTracer = std::map<Owner, CallStack>;

	Object* object_;
	BackTracer back_tracer_;
	const ObjectPool& bind_obj_pool_;
};

