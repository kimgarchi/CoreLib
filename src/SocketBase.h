#pragma once
#include <WinSock2.h>
#include "Object.h"

class SocketBase abstract : public object
{
public:
	SocketBase()
		: sock_(INVALID_SOCKET)
	{
		
	}

	virtual ~SocketBase() 
	{
		closesocket(sock_);
	};

	virtual bool Initialize() abstract;

private:
	
	SOCKET sock_;
};