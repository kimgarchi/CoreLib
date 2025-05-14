#include "stdafx.h"
#include "SocketBase.h"

SocketBase::SocketBase(const SOCKET sock, const SOCK_TYPE sock_type, const DWORD buf_size)
	: sock_{ sock }, buffer_{ allocate_vector<char>() }
{
	assert(sock_ != INVALID_SOCKET);
	memset(&sock_addr_, 0x00, sizeof(SOCKADDR_IN));
	buffer_.resize(buf_size, 0x00);
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
