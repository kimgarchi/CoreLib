#pragma once
#include "stdafx.h"
#include "Job.h"


class CompletionPort
{
public:
	CompletionPort(DWORD thread_count, JobBaseHub job);
	~CompletionPort();

private:
	using BindHandles = std::set<HANDLE>;

	TaskID task_id_;
	HANDLE completion_port_;
	BindHandles bind_handles_;
};