#pragma once
#include "stdafx.h"
#include "singleton.h"
#include "Sock.h"
#include "ThreadManager.h"


#define INVALID_POST_ID 0

enum class SOCK_TYPE
{
	TCP,
	UDP,
};

class CompletionPort;

using BindAllSocks = std::set<SOCKET>;
using PostID = size_t;
using CompletionPorts = std::map<PostID, CompletionPort>;

class PostCenter : public Singleton<PostCenter>
{
private:
	using ReserveQue = std::queue<SOCKET>;
	using BindPorts = std::map<PostID, USHORT>;
	
public:
	using AttachJobs = std::map<TaskID, JobBaseHub>;

	PostCenter();
	~PostCenter();

	PostID RecordCompletionPort(SOCK_TYPE type, USHORT port, int wait_que_size, DWORD thread_count);
	
private:
	bool BindSocket(PostID post_id, SOCKET sock);

	CompletionPorts completion_ports_;
	BindAllSocks bind_all_socks_;

	AttachJobs attach_jobs_;

	std::atomic<PostID> alloc_post_id_;
};