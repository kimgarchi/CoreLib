#pragma once

#include "SocketBase.h"

using PORT = USHORT;
class IocpSock : public SocketBase, public WSAOVERLAPPED
{
public:
	IocpSock(DWORD buf_size = DEFAULT_BUF_SIZE)
		: SocketBase(buf_size)
	{
		Init();

		wsa_buf_.len = static_cast<ULONG>(buffer_.size());
		wsa_buf_.buf = &buffer_.at(0);
	}

	IocpSock(SOCKET sock, SOCKADDR_IN sock_addr, DWORD buf_size = DEFAULT_BUF_SIZE)
		: SocketBase(sock, sock_addr, buf_size)
	{
		Init();

		wsa_buf_.len = static_cast<ULONG>(buffer_.size());		
		wsa_buf_.buf = &buffer_.at(0);
	}

	virtual ~IocpSock()
	{
	}

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

	WSABUF& wsa_buf() { return wsa_buf_; }
	
private:
	WSABUF wsa_buf_;
};