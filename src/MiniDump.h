#pragma once
#include "stdafx.h"
#include "dbghelp.h"
#include <tchar.h>

typedef BOOL(WINAPI* MINIDUMPWRITEDUMP)
(
	HANDLE hProcess,
	DWORD dwPid,
	HANDLE hFile,
	MINIDUMP_TYPE DumpType,
	const PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	const PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	const PMINIDUMP_CALLBACK_INFORMATION callbackParam);

static LONG WINAPI UnHandledExceptionFilter(struct _EXCEPTION_POINTERS* exceptionInfo)
{
	HMODULE dllHandle = nullptr;

#ifdef UNICODE
	dllHandle = LoadLibrary(L"DBGHELP.DLL");
#else
	dllHandle = LoadLibrary(_T("DBGHELP.DLL"));
#endif

	if (dllHandle != nullptr)
	{
		MINIDUMPWRITEDUMP dump = (MINIDUMPWRITEDUMP)GetProcAddress(dllHandle, "MiniDumpWriteDump");
		if (dump != nullptr)
		{
			SYSTEMTIME system_time;
			GetLocalTime(&system_time);

#ifdef UNICODE
			std::wstringstream dump_path_stream;
			wchar_t path_hyphen = L'-';
			const wchar_t* file_type = L".dmp";
#else
			std::stringstream dump_path_stream;
			const char* path_hyphen = _T("-");
			const char* file_type = _T(".dmp");
#endif
			dump_path_stream << system_time.wYear << path_hyphen;
			dump_path_stream << system_time.wMonth << path_hyphen;
			dump_path_stream << system_time.wDay << path_hyphen;
			dump_path_stream << system_time.wHour << path_hyphen;
			dump_path_stream << system_time.wMinute << path_hyphen;
			dump_path_stream << system_time.wSecond << file_type;

#ifdef UNICODE
			std::wstring dump_path = dump_path_stream.str();
			LPCWSTR path = dump_path.c_str();
#else
			std::string dump_path = dump_path_stream.str();
			LPCSTR path = dump_path.c_str();
#endif
			HANDLE file_handle = CreateFile(path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (file_handle != INVALID_HANDLE_VALUE)
			{
				_MINIDUMP_EXCEPTION_INFORMATION minidump_exception_info;

				minidump_exception_info.ThreadId = GetCurrentThreadId();
				minidump_exception_info.ExceptionPointers = exceptionInfo;
				minidump_exception_info.ClientPointers = NULL;

				BOOL success = dump(
					GetCurrentProcess(),
					GetCurrentProcessId(),
					file_handle,
					MiniDumpNormal,
					&minidump_exception_info,
					NULL,
					NULL
				);

				if (success)
				{
					CloseHandle(file_handle);

					return EXCEPTION_EXECUTE_HANDLER;
				}
			}

			CloseHandle(file_handle);
		}
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

static VOID WINAPI AttachExceptionHandler()
{
	SetUnhandledExceptionFilter(UnHandledExceptionFilter);
}