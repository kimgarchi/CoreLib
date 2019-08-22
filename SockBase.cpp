#pragma once
#include "SocketBase.h"

SocketBase::SocketBase()
	: sock_(INVALID_SOCKET), type_(SOCKET_TYPE::NONE)
{
	
}

SocketBase::~SocketBase()
{
}