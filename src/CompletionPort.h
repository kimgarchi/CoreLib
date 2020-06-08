#pragma once
#include "stdafx.h"
#include "Job.h"

class CompletionJob : public JobBase
{
public:
	CompletionJob(HANDLE completion_handle);

	virtual bool Work(PVOID key, OVERLAPPED& overlapped) abstract;

private:
	virtual bool Work() override;

	HANDLE completion_handle_;
};

DEFINE_WRAPPER_HUB(CompletionJob);
DEFINE_WRAPPER_NODE(CompletionJob);

class CompletionPort
{
public:
	CompletionPort(DWORD thread_count, CompletionJobHub job);
	~CompletionPort();

	bool AttachHandle(HANDLE handle, PVOID key);

private:
	HANDLE completion_port_handle_;
	TaskID task_id_;
};

