#pragma once

#include "SocketBase.h"

using PORT = USHORT;
class IocpSock : public SocketBase
{
public:
	IocpSock(SOCKET sock, SOCKADDR_IN sock_addr)
		: SocketBase(sock, sock_addr)
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
		memset(&overlapped_, 0x00, sizeof(OVERLAPPED));

		if (reset_size == 0 || reset_size >= buffer_.size())
			std::fill(buffer_.begin(), buffer_.end(), 0x00);
		else
			std::fill(buffer_.begin(), buffer_.begin() + reset_size, 0x00);
	}

	WSABUF& wsa_buf() { return wsa_buf_; }
	OVERLAPPED& overlapped() { return overlapped_; }

private:
	OVERLAPPED overlapped_;
	WSABUF wsa_buf_;	
};

using IocpSockHub = wrapper_hub<IocpSock>;
using IocpSockNode = wrapper_node<IocpSock>;