#pragma once
#include "stdafx.h"
#include "Sock.h"


#define ATTACH_TYPE_ACCEPT	0
#define ATTACH_TYPE_INOUT	1

#define ACCEPT_RECV_SIZE 16

/*
class CompletionPort
{
public:
	CompletionPort(SOCKET sock, USHORT port, int wait_que_size, DWORD thread_count);
	virtual ~CompletionPort();

	inline const SOCKET sock() { return sock_; }
	inline const HANDLE comp_port() { return comp_port_; }

	bool AttachSock(const SOCKET clnt_sock);
	void DeattachSock(SOCKET sock);

	bool AttachKey(BYTE type, PVOID key);

	IocpSock* GetIocpSock(SOCKET sock);

	void InitFunc();
	void ExtendSocketPool(size_t count);

private:
	using BIndSockPair = std::pair<SOCKET, std::shared_ptr<IocpSock>>;
	using BindSocks = std::map<SOCKET, std::shared_ptr<IocpSock>>;

	HANDLE comp_port_;
	SOCKET sock_;
	SOCKADDR_IN sock_addr_;
	BindSocks bind_socks_;

	LPFN_ACCEPTEX pfn_acceptex_;
	LPFN_DISCONNECTEX pfn_disconnectex_;
};
*/