#include "stdafx.h"
#include "CompletionPort.h"
#include "ThreadManager.h"

CompletionPort::CompletionPort(DWORD thread_count, JobBaseHub job)
	: task_id_(INVALID_ALLOC_ID), completion_port_(INVALID_HANDLE_VALUE)
{
	task_id_ = ThreadManager::GetInstance().AttachTask(thread_count, job);
	if (task_id_ != INVALID_ALLOC_ID)
		return;

	throw std::runtime_error("completion port task attach failed");
}

CompletionPort::~CompletionPort()
{
	CloseHandle(completion_port_);
}
