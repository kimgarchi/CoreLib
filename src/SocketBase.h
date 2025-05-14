#pragma once
#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib,"mswsock.lib")

#include <WinSock2.h>
#include <mswsock.h>

#define DEFAULT_BUF_SIZE 512

enum class SOCK_TYPE
{
	TCP,
	UDP,
};

class SocketBase abstract : public NonCopyableBase
{
public:
	SocketBase(SOCKET sock, const SOCK_TYPE sock_type, const DWORD buf_size);
	virtual ~SocketBase();

	SOCKET sock() const;
	const char* buffer();
	const size_t buffer_size() const;

protected:	
	m_vector<char> buffer_;

private:
	SOCKET sock_;
	SOCKADDR_IN sock_addr_;
};