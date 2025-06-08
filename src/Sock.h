#pragma once
#include "SocketBase.h"

class ClientSocket : public SocketBase
{
public:
	ClientSocket() = default;
	virtual ~ClientSocket() = default;

	bool connect();
	virtual DWORD recv();
	virtual DWORD send();

private:
	WSABUF wsa_buf_;
};

class ServerSocket : public SocketBase
{
public:
	ServerSocket(SOCKET sock, SOCKADDR_IN sock_addr)
}

class IocpSock : public SocketBase, public WSAOVERLAPPED
{
public:
	IocpSock(SOCKET sock, SOCK_TYPE sock_type, DWORD buf_size = DEFAULT_BUF_SIZE)
		: SocketBase(sock, sock_type, buf_size)
	{
		Init();
	}

	virtual ~IocpSock() = default;

	void Init(size_t reset_size = 0)
	{
		hEvent			= 0x00;
		Internal		= 0x00;
		InternalHigh	= 0x00;
		Offset			= 0x00;
		OffsetHigh		= 0x00;

		if (reset_size == 0 || reset_size >= buffer_.size())
			std::fill(buffer_.begin(), buffer_.end(), 0x00);
		else
			std::fill(buffer_.begin(), buffer_.begin() + reset_size, 0x00);
	}
};