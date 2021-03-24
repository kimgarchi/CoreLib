#pragma once
#include "stdafx.h"
#include "Object.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>

#define DEFAULT_BUF_SIZE 512

class SocketBase abstract : public object
{
public:
	SocketBase(SOCKET sock, SOCKADDR_IN sock_addr, DWORD default_buf_size = DEFAULT_BUF_SIZE)
		: sock_(sock), sock_addr_(sock_addr), buffer_(DEFAULT_BUF_SIZE, 0x00)
	{
		//memset(&buffer_, 0x00, buffer_.size());
	}

	virtual ~SocketBase() 
	{
		if (sock_ != INVALID_SOCKET)
			closesocket(sock_);
	};

	SOCKET sock() { return sock_; }
	const char* buffer() { return &buffer_.at(0); }

protected:
	using Buffer = std::vector<char>;

	Buffer buffer_;

private:
	const SOCKET sock_;
	const SOCKADDR_IN sock_addr_;
};