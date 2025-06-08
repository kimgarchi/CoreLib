#include "stdafx.h"
#include "Sock.h"

bool ClientSocket::connect()
{
	if (sock_ == INVALID_SOCKET)
		return false;

	DWORD ret = WSAConnect(
		sock_,
		(LPSOCKADDR)&sock_addr_, sizeof(sock_addr_), 
		NULL, NULL, NULL, NULL);

	/*
	* https://learn.microsoft.com/ko-kr/windows/win32/api/winsock2/nf-winsock2-wsaconnect
	*/

	if (ret != 0)
	{
		switch (ret)
		{
		case WSANOTINITIALISED:
			break;
		case WSAENETDOWN:
			break;
		case WSAEADDRINUSE:
			break;
		case WSAEISCONN:
			break;
		case WSAENETUNREACH:
			break;
		case WSAEHOSTUNREACH:
			break;
		case WSAENOBUFS:
			break;
		default:
			break;
		}

		return false;
	}


	return true;	
}

DWORD ClientSocket::recv()
{
	return 0;
}

DWORD ClientSocket::send()
{
	return 0;
}
