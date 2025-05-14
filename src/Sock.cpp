#include "stdafx.h"
#include "Sock.h"

WsaSock::WsaSock(SOCKET sock, const SOCK_TYPE sock_type, const DWORD buf_size)
	: SocketBase(sock, sock_type, buf_size)
{
	wsa_buf_.len = static_cast<ULONG>(buffer_.size());
	wsa_buf_.buf = &buffer_.at(0);
}
