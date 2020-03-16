#pragma once
#include "stdafx.h"
#include "singleton.h"
#include "SyncObject.h"
#include "Job.h"

class SyncStation : public Singleton<SyncStation>
{
private:
	enum class HANDLE_STATE
	{
		IDLE,
		READ_LOCK,
		WRITE_LOCK
	};

	class RWHandle
	{
	public:
		RWHandle(LONG read_job_init_count, LONG read_job_max_count);
		decltype(auto) state();		
		inline decltype(auto) WriteHandle() { return mutex_.get(); }
		inline decltype(auto) Readhandle() { return semaphore_.get(); }

	private:
		SyncMutexHub mutex_;
		SyncSemaphoreHub semaphore_;
	};

	using HandleByType = std::unordered_map<TypeID, RWHandle>;
	using Handles = std::vector<HANDLE>;
	using HandleState = std::unordered_map<TypeID, HANDLE_STATE>;

public:
	template<typename ... _Tys>
	bool RegistReadJob(JobBaseHub job);

	template<typename ... _Tys>
	bool RegistWriteJob(JobBaseHub job);
	
private:
	bool RecordHandle(TypeID tid, LONG read_job_init_count, LONG read_job_max_count);
	HANDLE_STATE handle_state(TypeID tid);
	bool IsRecordType(TypeID tid);

	inline decltype(auto) ReadHandles() { return read_handles_.data(); }
	inline decltype(auto) WriteHandles() { return write_handles_.data(); }

	HandleByType handle_by_type_;
	Handles read_handles_;
	Handles write_handles_;

	std::shared_mutex distribute_mtx_;
	std::mutex depot_mtx_;
};

template<typename ..._Tys>
bool SyncStation::RegistReadJob(JobBaseHub job)
{
	return false;
}

template<typename ..._Tys>
bool SyncStation::RegistWriteJob(JobBaseHub job)
{
	return false;
}
