#pragma once
#include "stdafx.h"
#include "SyncObject.h"

class SyncOverlapped
{
public:
	SyncOverlapped(HANDLE handle = INVALID_HANDLE_VALUE);

private:

	OVERLAPPED overlapped_;
};

