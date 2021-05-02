#pragma once
#include "stdafx.h"
#include "Sock.h"


#define ATTACH_TYPE_ACCEPT	0
#define ATTACH_TYPE_INOUT	1


class CompletionPort : public object
{
public:
	CompletionPort(SOCKET sock, USHORT port, int wait_que_size, DWORD thread_count);
	virtual ~CompletionPort();

	inline const SOCKET sock() { return sock_; }
	inline const HANDLE comp_port() { return comp_port_; }

	bool AttachSock(IocpSockHub sock_hub);
	void DeattachSock(SOCKET sock);

	bool AttachKey(BYTE type, PVOID key);

	IocpSock* GetIocpSock(SOCKET sock);

private:
	using BindSocks = std::map<SOCKET, IocpSockHub>;

	HANDLE comp_port_;
	SOCKET sock_;
	SOCKADDR_IN sock_addr_;
	BindSocks bind_socks_;
};

DEFINE_WRAPPER(CompletionPort);