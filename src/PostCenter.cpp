#include "stdafx.h"
#include "PostCenter.h"
#include "CompletionPort.h"

PostCenter::PostCenter()
{
	static std::once_flag global_once_flag;

	std::call_once(global_once_flag,
		[]() {			
			WSADATA wsaData;
			// required ret code 0
			auto ret_code = WSAStartup(MAKEWORD(2, 2), &wsaData);
			switch (ret_code)
			{
			case WSASYSNOTREADY:
				// 기본 네트워크 하위 시스템은 네트워크 통신에 사용할 준비가 되지 않았습니다.
			case WSAVERNOTSUPPORTED:
				// 요청된 Windows 소켓 지원 버전은 이 특정 Windows 소켓 구현에서 제공되지 않습니다.
			case WSAEINPROGRESS:
				// 차단 Windows 소켓 1.1 작업이 진행 중입니다.
			case WSAEPROCLIM:
				// Windows 소켓 구현에서 지원하는 작업 수에 대한 제한에 도달했습니다.
			case WSAEFAULT:
				// _lpWSAData_ 매개 변수가 유효한 포인터가 아닙니다.
			default:
				// 예상되지 않은 실패 규격
				throw std::runtime_error("WSAStartup");
			}
		});
}

PostID PostCenter::RecordCompletionPort(SOCK_TYPE type, USHORT port, int wait_que_size, DWORD thread_count)
{
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
		return 0;

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