#include "stdafx.h"
#include "PostCenter.h"

PostCenter::PostCenter()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		std::bad_exception{};
}

PostCenter::~PostCenter()
{
}

PostID PostCenter::RecordCompletionPort(Func func, DWORD thread_count)
{
	PostID post_id = alloc_post_id_.fetch_add(1);
	if (completion_ports_.find(post_id) != completion_ports_.end())
		return INVALID_POST_ID;

	return true;
}

HANDLE PostCenter::NewCompletionPort(DWORD thread_count)
{
	return CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, thread_count);
}

PostCenter::CompletionPort::CompletionPort(Func func, DWORD thread_count)
{
}

PostCenter::CompletionPort::~CompletionPort()
{
}

bool PostCenter::CompletionPort::BindSock(SOCKET sock)
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
