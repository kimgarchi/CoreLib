#pragma once
#include <WinSock2.h>
#include "Object.h"

enum class SOCKET_TYPE
{
	NONE,
	TCP,
	UDP_UNCONNECTED,
	UDP_CONNECTED,
};

class SocketBase : public Object
{
public:
	SocketBase();
	virtual ~SocketBase() {};

	virtual bool Initialize() override {};

private:
	
	SOCKET_TYPE type_;
	SOCKET sock_;
};

SocketBase::SocketBase()
	: type_(SOCKET_TYPE::NONE)
{
}

SocketBase::~SocketBase()
{
}

/*
WSADATA wsaData;
	BYTE nMajor = 2;
	BYTE nMinor = 2;
	int i = 0;

	WORD wVersionRequested = MAKEWORD(nMinor, nMajor);

	int nRet = WSAStartup(wVersionRequested, &wsaData);
	if (nRet == SOCKET_ERROR)
	{
		//...
		return -1;
	}

	if (LOBYTE(wsaData.wVersion) != nMajor ||
		HIBYTE(wsaData.wVersion) != nMinor)
	{
		//...

		return -1;
	}

	SOCKET lstnsock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	try
	{
		if (lstnsock == INVALID_SOCKET)
		{
			throw "wait socket create failed";
		}

		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(42001);
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		if (bind(lstnsock, (struct sockaddr*) & addr, sizeof(addr)) == SOCKET_ERROR)
		{
			throw "bind error";
		}

		if (listen(lstnsock, SOMAXCONN) == SOCKET_ERROR)
		{
			throw "listen error";
		}

		while (true)
		{
			struct sockaddr_in cliaddr;
			int addrlen = sizeof(cliaddr);

			SOCKET commsock = accept(lstnsock, (struct sockaddr*) & cliaddr, &addrlen);

			time_t long_time;
			struct tm* newtime;
			char sztime[100];

			time(&long_time);
			newtime = localtime(&long_time);

			sprintf(sztime, ".%19s\n", asctime(newtime));
			send(commsock, sztime, (int)strlen(sztime), 0);
			closesocket(commsock);
		}
	}
	catch (char* errmsg)
	{
		std::cout << errmsg << std::endl;

		LPVOID lpOSMsg;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, WSAGetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)& lpOSMsg, 0, NULL);

		std::cout << (char*)lpOSMsg << std::endl;
		LocalFree(lpOSMsg);
	}

	WSACleanup();


*/