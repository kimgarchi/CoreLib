#include "stdafx.h"
#include "PostCenter.h"
#include "JobStation.h"
#include "CompletionPort.h"

PostCenter::PostCenter()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		throw std::runtime_error("WSAStartup");
}

PostCenter::~PostCenter()
{
}

PostID PostCenter::RecordCompletionPort(SOCK_TYPE type, USHORT port, int wait_que_size, DWORD thread_count)
{
	std::unique_lock<std::mutex> lock(mtx_);
	const auto post_id = alloc_post_id_.fetch_add(1);
	SOCKET sock = INVALID_SOCKET;

	switch (type)
	{
	case SOCK_TYPE::TCP:
		sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		break;
	case SOCK_TYPE::UDP:
		sock = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		break;
	}

	auto ret = completion_ports_.try_emplace(post_id, sock, port, wait_que_size, thread_count);
	if (ret.second == false)
		return INVALID_ALLOC_ID;

	auto& compe_itor = ret.first;

	return compe_itor->first;
}

bool PostCenter::BindSocket(PostID post_id, SOCKET sock)
{
	if (completion_ports_.find(post_id) != completion_ports_.end())
		return false;

	if (bind_all_socks_.find(sock) != bind_all_socks_.end())
		return false;

	if (bind_all_socks_.emplace(sock).second == false)
		return false;

	return true;
}