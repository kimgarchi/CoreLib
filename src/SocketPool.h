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
		if (WSAStartup(MAKEWORD(2, 2), &wsa_data_))
			throw std::runtime_error("WSAStartup Failed");
	}

	~SocketPool()
	{
		WSACleanup();
	}

private:
	WSADATA wsa_data_;
};

