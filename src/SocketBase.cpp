#include "stdafx.h"
#include "SocketBase.h"

SocketBase::SocketBase(const SOCK_TYPE sock_type, const DWORD buf_size)
	: sock_(INVALID_SOCKET), buffer_(allocate_vector<char>())
{
	switch (sock_type)
	{
	case SOCK_TYPE::TCP:
		sock_ = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		break;
	case SOCK_TYPE::UDP:
		sock_ = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		break;
	}

	assert(sock_ != INVALID_SOCKET);
	memset(&sock_addr_, 0x00, sizeof(SOCKADDR_IN));
}

SocketBase::~SocketBase()
{
	if (sock_ != INVALID_SOCKET)
		closesocket(sock_);
}

SOCKET SocketBase::sock() const
{
	return sock_;
}

const char* SocketBase::buffer()
{
	return &buffer_.at(0);
}

const size_t SocketBase::buffer_size() const
{
	return buffer_.size();
}
