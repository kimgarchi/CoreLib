#pragma once

class Object
{
public:
	Object() {};
	virtual ~Object() {};

	virtual bool Initialize() = 0;

private:
	friend class ObjectPool;
};