#include "stdafx.h"
#include "PostCenter.h"
#include "JobStation.h"

PostCenter::PostCenter()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		std::bad_exception{};
}

PostCenter::~PostCenter()
{
}

PostCenter::CompletionPort::CompletionPort(TaskID task_id, SOCKET sock, USHORT port, int wait_que_size, DWORD thread_count)
	: task_id_(task_id), sock_(sock), comp_port_(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, thread_count))
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

PostCenter::CompletionPort::~CompletionPort()
{
}

bool PostCenter::CompletionPort::AttachSock(SOCKET sock)
{

	return false;
}

bool PostCenter::CompletionPort::DeattachSock(SOCKET sock)
{

	return false;
}

bool PostCenter::BindSocket(PostID post_id, SOCKET sock)
{
	if (completion_ports_.find(post_id) != completion_ports_.end())
		return false;

	if (bind_all_socks_.find(sock) != bind_all_socks_.end())
		return false;

	return false;
}

PostCenter::SockAcceptJob::SockAcceptJob(CompletionPort& compe_port)
	: compe_port_(compe_port)
{
}

PostCenter::SockAcceptJob::~SockAcceptJob()
{
}

bool PostCenter::SockAcceptJob::Prepare()
{
	return false;
}

bool PostCenter::SockAcceptJob::RepeatWork()
{
	
	return false;
}
