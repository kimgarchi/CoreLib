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