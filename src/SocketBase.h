#pragma once
#include <WinSock2.h>
#include "Object.h"

#define BUF_SIZE 256

class SocketBase abstract
{
public:
	SocketBase(SOCKET sock, SOCKADDR_IN sock_addr)
		: sock_(sock), sock_addr_(sock_addr)
	{}

	virtual ~SocketBase() 
	{
		if (sock_ != INVALID_SOCKET)
			closesocket(sock_);
	};

	SOCKET sock() { return sock_; }

private:	
	SOCKET sock_;
	SOCKADDR_IN sock_addr_;
};

class IocpSock : public SocketBase
{
public:
	IocpSock(SOCKET sock, SOCKADDR_IN sock_addr)
		: SocketBase(sock, sock_addr)
	{
		memset(&overlapped_, 0x00, sizeof(OVERLAPPED));
		memset(buffer, 0x00, BUF_SIZE);
		wsa_buf_.len = sizeof(buffer);
		wsa_buf_.buf = buffer;
	}
	
	virtual ~IocpSock()
	{
	}

	WSABUF& wsa_buf() { return wsa_buf_; }
	OVERLAPPED& overlapped() { return overlapped_; }

private:
	OVERLAPPED overlapped_;
	WSABUF wsa_buf_;
	char buffer[BUF_SIZE];
};