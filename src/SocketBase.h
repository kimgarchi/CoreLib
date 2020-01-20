#pragma once
#include <WinSock2.h>
#include "Object.h"

class SocketBase abstract : public object
{
public:
	SocketBase(SOCKET sock)
		: sock_(sock)
	{}

	virtual ~SocketBase() 
	{
		closesocket(sock_);
	};

	virtual bool Initialize() abstract;

private:
	
	SOCKET sock_;
};