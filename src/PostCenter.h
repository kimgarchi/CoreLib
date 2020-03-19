#pragma once
#include "stdafx.h"
#include "singleton.h"
#include "SocketBase.h"
#include "ThreadManager.h"

#define INVALID_POST_ID 0

enum class SOCK_TYPE
{
	TCP,
	UDP,
};

using BindAllSocks = std::set<SOCKET>;
using PostID = size_t;

class PostCenter : public Singleton<PostCenter>
{
private:
	using ReserveQue = std::queue<SOCKET>;
	using BindSocks = std::map<SOCKET, IocpSock>;
	
	class SockAcceptJob;
	class CompletionPort
	{
	public:
		CompletionPort(TaskID task_id, SOCKET sock, USHORT port, int wait_que_size, DWORD thread_count);
		~CompletionPort();

	private:
		friend class SockAcceptJob;

		bool AttachSock(SOCKET sock);
		bool DeattachSock(SOCKET sock);

		HANDLE comp_port_;
		SOCKET sock_;
		SOCKADDR_IN sock_addr_;
		BindSocks bind_socks_;
		TaskID task_id_;

		std::mutex mtx_;
	};

	class SockAcceptJob : public JobBase
	{
	public:
		SockAcceptJob(CompletionPort& compe_port __in);
		virtual ~SockAcceptJob();

		virtual bool RepeatWork() override;

	private:
		CompletionPort& compe_port_;
	};

	using CompletionPorts = std::map<PostID, CompletionPort>;

public:
	PostCenter();
	~PostCenter();

	template<typename _Job, typename ..._Tys, is_job<_Job> = nullptr>
	PostID RecordCompletionPort(SOCK_TYPE type, USHORT port, int wait_que_size, DWORD thread_count, _Tys&&... Args);
	bool BindSocket(PostID post_id, SOCKET sock);

private:
	CompletionPorts completion_ports_;
	BindAllSocks bind_all_socks_;
	std::atomic<PostID> alloc_post_id_;
	std::mutex mtx_;
};

template<typename _Job, typename ..._Tys, is_job<_Job>>
PostID PostCenter::RecordCompletionPort(SOCK_TYPE type, USHORT port, int wait_que_size, DWORD thread_count, _Tys&& ...Args)
{
	std::unique_lock<std::mutex> lock(mtx_);
	TaskID task_id = ThreadManager::GetInstance().AttachTask<_Job>(thread_count, Args...);
	if (task_id == INVALID_ALLOC_ID)
		return INVALID_ALLOC_ID;
	
	PostID post_id = alloc_post_id_.fetch_add(1);
	SOCKET sock = INVALID_SOCKET;

	switch (type)
	{
	case SOCK_TYPE::TCP:
	{
		sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	}
	break;
	case SOCK_TYPE::UDP:
	{
		sock = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	}
	break;
	}
	
	completion_ports_.emplace(post_id, task_id, sock, port, wait_que_size, thread_count);

	return post_id;
}