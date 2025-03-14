#pragma once
#include "stdafx.h"
#include "SyncObject.h"

#ifdef _DEBUG
const static size_t _default_read_count_ = 20;
#else
const static size_t _default_read_count_ = 50;
#endif

class SyncStation
{
private:
	using HandleByType = std::unordered_map<std::type_index, std::shared_ptr<SyncSemaphore>>;
	using Handles = std::vector<HANDLE>;
	
public:
	SyncStation();
	virtual ~SyncStation();

private:
	HANDLE RecordHandle(std::type_index type_idx, LONG read_job_max_count = _default_read_count_);
	bool IsRecordType(std::type_index type_idx) { return (handle_by_type_.find(type_idx) != handle_by_type_.end()); }

	HandleByType handle_by_type_;
	std::shared_ptr<SyncMutex> mutex_ptr_;
	std::shared_ptr<SyncEvent> event_ptr_;
	std::vector<std::size_t> worker_task_ids_;
};