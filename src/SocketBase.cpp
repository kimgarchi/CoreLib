#include "stdafx.h"
#include "SocketBase.h"

SocketBase::~SocketBase()
{
	if (sock_ != INVALID_SOCKET)
		closesocket(sock_);
}

bool SocketBase::InitSocket(const BYTE SOCK_TYPE)
{
	switch (SOCK_TYPE)
	{
	case SOCK_STREAM:
	case SOCK_DGRAM:
		sock_ = WSASocket(AF_INET, SOCK_TYPE, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		break;
	}

	if (sock_ == INVALID_SOCKET)
		return false;

	return false;
}

bool SocketBase::setSockOption(int level, int option, BYTE value)
{
	if (sock_ == INVALID_SOCKET)
		return false;
	//ioctlsocket
	/*
	* https://learn.microsoft.com/ko-kr/windows/win32/api/winsock2/nf-winsock2-setsockopt
	*/

	/*
	* level
	* SOL_SOCKET
	* IPPROTO_TCP
	*/

	setsockopt(sock_, level, option, (char*) & value, sizeof(value));

	return true;
}

bool SocketBase::InitSocketAddr(const std::string& addr, const WORD port)
{
	if (addr.empty())
		return false;

	sock_addr_.sin_family = AF_INET;
	sock_addr_.sin_port = htons(port);
	sock_addr_.sin_addr.S_un.S_addr = inet_addr(addr.c_str());
	sock_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
	
	return true;
}
