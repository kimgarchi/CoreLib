#pragma once

enum class HANDLE_STATE
{
	NONE,
	IDLE,
	READ_LOCK,
	WRITE_LOCK,
};