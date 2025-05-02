#pragma once
#include "stdafx.h"
#include "CompletionPort.h"

CompletionPort::CompletionPort(SOCKET sock, USHORT port, int wait_que_size, DWORD thread_count)
	: sock_(sock), comp_port_(INVALID_HANDLE_VALUE), pfn_acceptex_(NULL), pfn_disconnectex_(NULL)
{
	sock_addr_.sin_family = AF_INET;
	sock_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
	sock_addr_.sin_port = htons(port);

	if (bind(sock_, (SOCKADDR*)&sock_addr_, sizeof(sock_addr_)) != 0)
		assert(false);
	
	if (listen(sock_, wait_que_size) != 0)
		assert(false);
	
	comp_port_ = CreateIoCompletionPort((HANDLE)sock_, NULL, ATTACH_TYPE_ACCEPT, thread_count);

	InitFunc();
	ExtendSocketPool(2);

	//bind_socks_ = allocate_map<SOCKET, std::shared_ptr<IocpSock>>();
}

CompletionPort::~CompletionPort()
{
	assert(CloseHandle(comp_port_));
}

bool CompletionPort::AttachSock(const std::shared_ptr<IocpSock>& sock_ptr)
{
	if (sock_ptr == nullptr)
		return false;

	if (AttachKey(ATTACH_TYPE_ACCEPT, (PVOID)(sock_ptr->sock())) == false)
		return false;

	return bind_socks_.emplace(sock_ptr->sock(), sock_ptr).second;
}

void CompletionPort::DeattachSock(SOCKET sock)
{
	IocpSock* iocp_sock = GetIocpSock(sock);
	if (iocp_sock == nullptr)
		return;

	//bind_socks_.erase(sock);
	pfn_disconnectex_(iocp_sock->sock(), NULL, TF_REUSE_SOCKET, 0);

	if (pfn_acceptex_(sock_, iocp_sock->sock(),
		iocp_sock->wsa_buf().buf, 0,
		sizeof(SOCKADDR_IN) + ACCEPT_RECV_SIZE, sizeof(SOCKADDR_IN) + ACCEPT_RECV_SIZE,
		NULL, &*iocp_sock) == false)
		return;
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

void CompletionPort::InitFunc()
{
	GUID guid = WSAID_ACCEPTEX;
	DWORD recvBytes = 0;
	auto ret = WSAIoctl(sock_, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(GUID), 
		&pfn_acceptex_, sizeof(LPFN_ACCEPTEX), &recvBytes, NULL, NULL);
	if (ret)
	{
		assert(false);
		return;
	}

	guid = WSAID_DISCONNECTEX;
	ret = WSAIoctl(sock_, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(GUID), 
		&pfn_disconnectex_, sizeof(LPFN_DISCONNECTEX), &recvBytes, NULL, NULL);
	if (ret)
	{
		assert(false);
		return;
	}
}

void CompletionPort::ExtendSocketPool(size_t count)
{
	if (pfn_acceptex_ == NULL)
		return;

	for (size_t idx = 0; idx < count; ++idx)
	{
		std::shared_ptr<IocpSock> sock_ptr = nullptr;//allocate_shared<IocpSock>();
		if (sock_ptr == nullptr)
			assert(false);

		if (pfn_acceptex_(sock_, sock_ptr->sock(),
			sock_ptr->wsa_buf().buf, 0,
			sizeof(SOCKADDR_IN) + ACCEPT_RECV_SIZE, sizeof(SOCKADDR_IN) + ACCEPT_RECV_SIZE,
			NULL, &*sock_ptr) == false)
		{
			DWORD error_code = WSAGetLastError();
			switch (error_code)
			{
			case WSA_IO_PENDING:
				break;
			default:
				std::cout << error_code << std::endl;
				assert(false);
				break;
			}
		}

		if (AttachSock(sock_ptr) == false)
			assert(false);
	}
}
