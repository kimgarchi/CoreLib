#pragma once
#include "stdafx.h"
#include "singleton.h"
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

class SocketPool final : public Singleton<SocketPool>
{
public:
	SocketPool()
	{
		auto ret = WSAStartup(MAKEWORD(2, 2), &wsa_data_);
		if (ret != 0)
		{
			assert(false);
			std::exception{};
		}
	}

	~SocketPool()
	{
		WSACleanup();
	}

private:
	WSADATA wsa_data_;
};

