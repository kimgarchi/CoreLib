#pragma once
#include "stdafx.h"
#include "singleton.h"
#include "SyncObject.h"
#include "Job.h"


#ifdef _DEBUG
const static size_t _default_read_count_ = 20;
#else
const static size_t _default_read_count_ = 100;
#endif

enum class JOB_TYPE
{
	READ,
	WRITE
};

class SyncStation : public Singleton<SyncStation>
{
private:
	class RWHandle
	{
	public:
		RWHandle(size_t array_idx, LONG read_job_max_count);
		decltype(auto) state();		
		inline decltype(auto) WriteHandle() { return mutex_.get(); }
		inline decltype(auto) Readhandle() { return semaphore_.get(); }

	private:
		SyncMutexHub mutex_;
		SyncSemaphoreHub semaphore_;
		size_t array_idx_;
	};

	using HandleByType = std::unordered_map<TypeID, RWHandle>;
	using Handles = std::vector<HANDLE>;
	
	class DistributeJob : public JobBase
	{
	public:
		DistributeJob(SyncStation& sync_station);

		virtual bool Work() override;

	private:
		size_t ShiftQue();
		SyncStation& sync_station_;
	};

	class ReservePackage
	{
	public:
		ReservePackage(JOB_TYPE&& type, TypeIds&& tids, JobBaseHub&& job_hub)
			: type_(type), tids_(tids), job_hub_(job_hub)
		{}

	private:
		friend class SyncStation;
		friend class DistributeJob;

		JOB_TYPE type_;
		TypeIds tids_;
		JobBaseHub job_hub_;
	};

	using JobQueue = std::queue<ReservePackage>;

public:
	SyncStation();
	virtual ~SyncStation();

	template<typename ... _Tys>
	bool RegistJob(JOB_TYPE type, JobBaseHub job);

private:
	friend class DistributeJob;

	bool RecordHandle(TypeID tid, LONG read_job_max_count);
	SYNC_STATE handle_state(TypeID tid);
	bool IsRecordType(TypeID tid);

	inline decltype(auto) ReadHandles() { return read_handles_.data(); }
	inline decltype(auto) WriteHandles() { return write_handles_.data(); }

	JobQueue& reserve_job_que() { return reserve_job_que_; }
	JobQueue& wait_job_que() { return wait_job_que_; }

	_NODISCARD SyncMutexNode mutex_node() { return mutex_hub_.make_node(); }

	bool RegistJob(TypeIds tids, JobBaseHub job);

	HandleByType handle_by_type_;
	Handles read_handles_;
	Handles write_handles_;

	JobQueue reserve_job_que_;
	JobQueue wait_job_que_;

	SyncMutexHub mutex_hub_;
	TaskID task_id_;
};

template<typename ..._Tys>
bool SyncStation::RegistJob(JOB_TYPE type, JobBaseHub job)
{
	auto tids = TypeHarvest::GetInstance().harvest<_Tys...>();
	SingleLock single_lock(mutex_hub_);

	reserve_job_que_.emplace(type, tids, job);
	
	return true;
}