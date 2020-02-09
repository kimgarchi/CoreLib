#pragma once
#include "stdafx.h"
#include "singleton.h"
#include "SyncObject.h"

using MutexHub = wrapper_hub<sync::Mutex>;
using MutexNode = wrapper_node<sync::Mutex>;

using SemaphoreHub = wrapper_hub<sync::Semaphore>;
using SemaphoreNode = wrapper_hub<sync::Semaphore>;

class Job;
//using JobUnit = wrapper_node<Job>;

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
		RWHandle(WORD idx);			
		decltype(auto) state();		
		inline decltype(auto) WriteHandle() { return *mutex_.get(); }
		inline decltype(auto) Readhandle() { return *semaphore_.get(); }

	private:
		WORD idx_;
		MutexHub mutex_;
		SemaphoreHub semaphore_;
	};

	using HandleByType = std::unordered_map<TypeID, RWHandle>;
	using Handles = std::vector<HANDLE>;
	using Types = std::vector<TypeID>;
	using HandleState = std::unordered_map<TypeID, HANDLE_STATE>;

public:	
	//bool RegistReadJob(HarvestTypes types, JobUnit job_unit);
	//bool RegistWriteJob(HarvestTypes types, JobUnit job_unit);
	
private:
	bool RecordHandle(TypeID tid);
	HANDLE_STATE handle_state(TypeID tid);
	bool IsRecordType(TypeID tid);

	inline decltype(auto) ReadHandles() { return read_handles_.data(); }
	inline decltype(auto) WriteHandles() { return write_handles_.data(); }

	HandleByType handle_by_type_;
	Handles read_handles_;
	Handles write_handles_;
	Types types_;

	std::shared_mutex distribute_mtx_;
	std::mutex depot_mtx_;
};