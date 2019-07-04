#pragma once
#include "stdafx.h"

#include "Log.h"
#include <iostream>
using std::cout;


namespace {
	const char* logFileName = "MemoryPool_Log.txt";
	bool consoleOutput = true;
}

class LogFile
{
public:
	void log(const char* str);
	~LogFile();

friend LogFile& st_LogFile();
private:
	LogFile();
	FILE* logFile;
};

LogFile& st_LogFile()
{
	static LogFile logFile;
	return logFile;
}

LogFile::LogFile()
{
	 fopen_s(&logFile, logFileName, "wt");
}

LogFile::~LogFile()
{
	fclose(logFile);
}

void LogFile::log(const char* str)
{
	int sLen = static_cast<int>(strlen(str));
	if (sLen>=3 && strcmp(&str[sLen-3], "...")==0)
	{
		fprintf(logFile, "%s", str);
		if (consoleOutput) cout << str;
	}
	else
	{
		fprintf(logFile, "%s\n", str);
		if (consoleOutput) cout << str << '\n';
	}
	fflush(logFile);
}

void Log(const string& str)
{
	st_LogFile().log(str.c_str());
}

void Log(const char* str)
{
	st_LogFile().log(str);
}

void Log(const int n)
{
	Log(I2S(n));
}

void FatalError(const string& str, const char* fileName, const int line)
{
	string s = "FATAL_ERROR : " + str;
	st_LogFile().log(s.c_str());
	if (fileName)
	{
		s = string(" FILE : ") + fileName; st_LogFile().log(s.c_str());
	}
	if (line != -1)
	{
		s = string(" LINE : ") + I2S(line); st_LogFile().log(s.c_str());
	}
	// you may yell out something here
	cout << "Fatal Error : " << str << '\n';
	// MessageBox(NULL, str.c_str(), "Fatal Error", MB_OK|MB_ICONEXCLAMATION);

#ifndef _DEBUG
	exit(1); // quit the program in release mode only. (you still need stack tracing in debug mode)
#endif
}

void FatalError(const char* str, const char* fileName, const int line)
{
	string s(str);
	FatalError(s, fileName, line);
}

string I2S(int n)
{
	static char buffer[128];
	sprintf_s(buffer, "%d", n);
	return string(buffer);
}
