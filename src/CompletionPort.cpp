#pragma once
#include "stdafx.h"
#include "CompletionPort.h"

CompletionPort::CompletionPort(SOCKET sock, USHORT port, int wait_que_size, DWORD thread_count)
	: sock_(sock), comp_port_(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, thread_count))
{
	sock_addr_.sin_family = AF_INET;
	sock_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
	sock_addr_.sin_port = htons(port);

	if (bind(sock_, (SOCKADDR*)&sock_addr_, sizeof(sock_addr_)) != 0)
	{
		//...
		assert(false);
	}

	if (listen(sock_, wait_que_size) != 0)
	{
		//...
		assert(false);
	}
}

CompletionPort::~CompletionPort()
{
	assert(CloseHandle(comp_port_));
}

bool CompletionPort::AttachSock(IocpSockHub sock_hub)
{
	SOCKET sock = sock_hub->sock();
	HANDLE ret = CreateIoCompletionPort((HANDLE)sock, comp_port_, (ULONG_PTR)(sock), 0);
	if (ret == INVALID_HANDLE_VALUE)
		return false;

	return bind_socks_.emplace(sock_hub->sock(), sock_hub).second;
}

void CompletionPort::DeattachSock(SOCKET sock)
{
	bind_socks_.erase(sock);
}

IocpSock* CompletionPort::GetIocpSock(SOCKET sock)
{
	auto itor = bind_socks_.find(sock);
	if (itor == bind_socks_.end())
		return nullptr;

	return itor->second.get();
}
