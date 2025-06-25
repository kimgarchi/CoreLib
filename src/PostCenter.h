#pragma once
#include "stdafx.h"
#include "Sock.h"
#include "ThreadManager.h"


#define INVALID_POST_ID 0

/*
class CompletionPort;

using BindAllSocks = std::set<SOCKET>;
using PostID = size_t;
using CompletionPorts = std::map<PostID, CompletionPort>;

class IocpSock;
class PostCenter
{
public:
	PostCenter();
	~PostCenter() = default;

	PostID RecordCompletionPort(SOCK_TYPE type, USHORT port, int wait_que_size, DWORD thread_count);
	
private:
	std::shared_ptr<IocpSock> MakeSock();
	bool BindSocket(PostID post_id, SOCKET sock);

	CompletionPorts completion_ports_;
	BindAllSocks bind_all_socks_;

	std::atomic<PostID> alloc_post_id_;
};
*/