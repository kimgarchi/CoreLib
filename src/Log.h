#pragma once
#include "stdafx.h"


enum class LOG_TYPE
{
	DEBUG,
	NOTIFY,
	INFORMATION,
	WARNING,
	FATAL
};

void WriteLog(std::wstring log, ...)
{
	
}