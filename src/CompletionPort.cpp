#pragma once
#include "stdafx.h"
#include "CompletionPort.h"
#include "ThreadManager.h"

CompletionJob::CompletionJob()
	: completion_handle_(INVALID_HANDLE_VALUE)
{
}

bool CompletionJob::Work()
{
	if (completion_handle_ == INVALID_HANDLE_VALUE)
	{
		assert(false);
		return false;
	}

	PVOID key = NULL;
	LPOVERLAPPED overlapped = nullptr;
	DWORD completion_size = 0;

	auto ret = GetQueuedCompletionStatus(completion_handle_, &completion_size, (PULONG_PTR)&key, &overlapped, INFINITE);
	if (ret == false)
	{
		DWORD error_code = GetLastError();
		// ... show error code
		return false;
	}

	return Work(key, *overlapped);
}

CompletionPort::CompletionPort(DWORD thread_count, CompletionJobHub& job_hub)
	: completion_port_handle_(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, thread_count)), task_id_(INVALID_ALLOC_ID)
{
	if (completion_port_handle_ == INVALID_HANDLE_VALUE)
		throw std::runtime_error("CreateIoCompletionPort Failed");

	job_hub->completion_handle_ = this->completion_port_handle_;

	task_id_ = ThreadManager::GetInstance().AttachTask(thread_count, job_hub);
	if (task_id_ == INVALID_ALLOC_ID)
		throw std::runtime_error("CompletionPort Task Attach Failed");
}

CompletionPort::~CompletionPort()
{
	assert(CloseHandle(completion_port_handle_));
	assert(ThreadManager::GetInstance().DeattachTask(task_id_));	
}

bool CompletionPort::AttachHandle(HANDLE handle, PVOID key)
{
	auto ret = CreateIoCompletionPort(handle, completion_port_handle_, (ULONG_PTR)(key), 0);
	if (ret == nullptr || ret != completion_port_handle_)
		return false;

	return true;
}