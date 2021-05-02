#pragma once
#include "stdafx.h"
#include "CompletionPort.h"

CompletionPort::CompletionPort(SOCKET sock, USHORT port, int wait_que_size, DWORD thread_count)
	: sock_(sock), comp_port_(INVALID_HANDLE_VALUE)
{
	sock_addr_.sin_family = AF_INET;
	sock_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
	sock_addr_.sin_port = port;//htons(port);

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

	comp_port_ = CreateIoCompletionPort((HANDLE)sock_, NULL, ATTACH_TYPE_ACCEPT, thread_count);
}

CompletionPort::~CompletionPort()
{
	assert(CloseHandle(comp_port_));
}

bool CompletionPort::AttachSock(IocpSockHub sock_hub)
{
	if (AttachKey(ATTACH_TYPE_ACCEPT, (PVOID)(sock_hub->sock())) == false)
		return false;

	return bind_socks_.emplace(sock_hub->sock(), sock_hub).second;
}

void CompletionPort::DeattachSock(SOCKET sock)
{
	bind_socks_.erase(sock);
}

bool CompletionPort::AttachKey(BYTE type, PVOID key)
{
	switch (type)
	{
	case ATTACH_TYPE_INOUT:
		break;
	case ATTACH_TYPE_ACCEPT:
		return true;
	default:
		return false;
	}

	HANDLE ret = CreateIoCompletionPort(key, comp_port_, (ULONG_PTR)(type), 0);
	if (ret == INVALID_HANDLE_VALUE)
		return false;

	return true;
}

IocpSock* CompletionPort::GetIocpSock(SOCKET sock)
{
	auto itor = bind_socks_.find(sock);
	if (itor == bind_socks_.end())
		return nullptr;

	return itor->second.get();
}
