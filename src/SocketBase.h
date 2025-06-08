#pragma once
#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib,"mswsock.lib")

#include <WinSock2.h>
#include <mswsock.h>


class SocketBase abstract : public NonCopyableBase
{
public:
	SocketBase() = default;
	virtual ~SocketBase();
	
	bool InitSocket(const BYTE SOCK_TYPE);
	bool setSockOption(int level, int option, BYTE value);
	bool InitSocketAddr(const std::string& addr, const WORD port);

protected:
	SOCKET sock_{ INVALID_SOCKET };
	SOCKADDR_IN sock_addr_;
};