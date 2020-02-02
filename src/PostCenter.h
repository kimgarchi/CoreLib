#pragma once
#include "stdafx.h"
#include "Sock.h"
#include "singleton.h"
#include "SocketBase.h"

#define INVALID_POST_ID 0

using BindAllSocks = std::set<SOCKET>;
using PostID = size_t;

class PostCenter : public Singleton<PostCenter>
{
private:
	class CompletionPort
	{
	private:
		using BindSocks = std::map<SOCKET, IocpSock>;
		using BindThreadIds = std::set<DWORD>;
	public:
		CompletionPort(Func func, DWORD thread_count);
		~CompletionPort();
		bool BindSock(SOCKET sock);

	private:
		HANDLE handle_;
		BindThreadIds bind_thread_ids_;
		BindSocks bind_socks_;
	};

	using CompletionPorts = std::map<PostID, CompletionPort>;

public:
	PostCenter();
	~PostCenter();

	PostID RecordCompletionPort(Func func, DWORD thread_count);
	bool BindSocket(PostID post_id, SOCKET sock);

private:
	HANDLE NewCompletionPort(DWORD thread_count);


	CompletionPorts completion_ports_;
	BindAllSocks bind_all_socks_;
	static std::atomic<PostID> alloc_post_id_;
};

