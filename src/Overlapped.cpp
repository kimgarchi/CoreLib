#include "stdafx.h"
#include "Overlapped.h"

SyncOverlapped::SyncOverlapped(HANDLE handle)
{
	memset(&overlapped_, 0x00, sizeof(OVERLAPPED));
	if (handle != INVALID_HANDLE_VALUE)
		overlapped_.hEvent = handle;
}
