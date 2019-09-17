#pragma once
#include "stdafx.h"
#include "ObjectPool.h"

class ObjectManager
{
public:
	ObjectManager(DWORD chunk_size, DWORD extend_chunk_size, DWORD max_chunk_size) {}
	ObjectManager(const ObjectManager&) = delete;
	ObjectManager& operator= (const ObjectManager&) = delete;

	virtual ~ObjectManager() {}

	virtual bool initialize() {}
	
protected:
	
private:
	
};
