#pragma once
#include "stdafx.h"
#include "SyncObject.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib,"mswsock.lib")

#include <WinSock2.h>
#include <mswsock.h>

#define DEFAULT_BUF_SIZE 512

class SocketBase abstract
{
public:
	SocketBase(DWORD buf_size)
		: sock_(INVALID_SOCKET), buffer_(buf_size, 0x00)
	{
		sock_ = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		//sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		memset(&sock_addr_, 0x00, sizeof(SOCKADDR_IN));
	}

	SocketBase(SOCKET sock, SOCKADDR_IN sock_addr, DWORD buf_size)
		: sock_(sock), sock_addr_(sock_addr), buffer_(buf_size, 0x00)
	{
	}

	virtual ~SocketBase() 
	{
		if (sock_ != INVALID_SOCKET)
			closesocket(sock_);
	};

	SOCKET sock() { return sock_; }
	inline const char* buffer() { return &buffer_.at(0); }
	inline const size_t buffer_size() const { return buffer_.size(); }

protected:
	using Buffer = std::vector<char>;

	Buffer buffer_;

private:
	SOCKET sock_;
	SOCKADDR_IN sock_addr_;
};