#pragma once
#include "stdafx.h"
#include "Job.h"

class CompletionJob;
class CompletionPort;

DEFINE_WRAPPER_HUB(CompletionJob);
DEFINE_WRAPPER_NODE(CompletionJob);

DEFINE_WRAPPER_HUB(CompletionPort);
DEFINE_WRAPPER_NODE(CompletionPort);

class CompletionJob : public JobBase
{
public:
	CompletionJob();

	virtual bool Work(PVOID key, OVERLAPPED& overlapped) abstract;

private:
	friend class CompletionPort;

	virtual bool Work() override;
	HANDLE completion_handle_;
};

class CompletionPort : public object
{
public:
	CompletionPort(DWORD thread_count, CompletionJobHub& job_hub);	
	virtual ~CompletionPort();

	inline HANDLE handle() { return completion_port_handle_; }

private:
	bool AttachHandle(HANDLE handle, PVOID key);

	HANDLE completion_port_handle_;
	TaskID task_id_;
};