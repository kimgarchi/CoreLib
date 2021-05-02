#pragma once
#include "stdafx.h"
#include "singleton.h"
#include "SyncObject.h"
#include "Job.h"


#ifdef _DEBUG
const static size_t _default_read_count_ = 20;
#else
const static size_t _default_read_count_ = 50;
#endif

class SyncStation : public Singleton<SyncStation>
{
private:
	using HandleByType = std::unordered_map<TypeID, SyncSemaphoreHub>;
	using Handles = std::vector<HANDLE>;
	
public:
	SyncStation();
	virtual ~SyncStation();

private:
	HANDLE RecordHandle(TypeID tid, LONG read_job_max_count = _default_read_count_);
	bool IsRecordType(TypeID tid) { return (handle_by_type_.find(tid) != handle_by_type_.end()); }

	HandleByType handle_by_type_;
	SyncMutexHub mutex_hub_;
	SyncEventHub event_hub_;	
	TaskIDs worker_task_ids_;
};