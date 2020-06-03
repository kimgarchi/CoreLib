#pragma once
#include "stdafx.h"
#include "singleton.h"
#include "SyncObject.h"

#ifdef _DEBUG
const static size_t _default_handle_pool_size_ = 20;
#else
const static size_t _default_handle_pool_size_ = 50;
#endif

enum class HANDLE_TYPE
{
	MUTEX,
	SEMAPHORE,
	EVENT,
};

class HandlePool : public Singleton<HandlePool>
{
private:


public:
	HandlePool();
	~HandlePool();

	HANDLE RequestHandle(HANDLE_TYPE type);


private:



};