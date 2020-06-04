#include "stdafx.h"
#include "CompletionPort.h"
#include "ThreadManager.h"
#include "Job.h"

CompletionPort::CompletionPort(DWORD thread_count, CompletionJobHub job)
	: handle_(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, thread_count)),
	task_id_(INVALID_ALLOC_ID)
{
	task_id_ = ThreadManager::GetInstance().AttachTask(thread_count, job);
	if (task_id_ == INVALID_ALLOC_ID)
		throw std::runtime_error("CompletionPort Task Attach Failed...");
}

CompletionPort::~CompletionPort()
{
	assert(CloseHandle(handle_));
}

bool CompletionPort::AttachHandle(HANDLE handle, PVOID key)
{
	auto ret = CreateIoCompletionPort(handle, handle_, (ULONG_PTR)(key), 0);
	if (ret == nullptr || ret != handle_)
		return false;

	return true;
}

CompletionJob::CompletionJob(HANDLE completion_handle)
	: completion_handle_(completion_handle)
{
}

bool CompletionJob::Work()
{
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
