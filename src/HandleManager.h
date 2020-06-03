#pragma once
#include "stdafx.h"
#include "singleton.h"

enum class HANDLE_TYPE
{
	MUTEX,
	SEMAPHORE,
	EVENT,

	TCP_SOCKET,
	UDP_SOCKET,

	COMPLETION_PORT,
};

class HandleManager : public Singleton<HandleManager>
{
public:

private:
};

