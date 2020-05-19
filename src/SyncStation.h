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
	using HandleByType = std::unordered_map<TypeID, SyncSemaphoreHub>;
	using Handles = std::vector<HANDLE>;
	
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

	using JobQueue = std::deque<ReservePackage>;

	class DistributeJob : public JobBase
	{
	public:
		DistributeJob(Handles& handles, JobQueue& reserve_job_que, SyncMutexNode mutex_node, SyncEventNode event_node);
		virtual bool Work() override;

	private:
		using HandleWait = std::vector<bool>;
		
		size_t ShiftQue();
		
		HandleWait handle_wait_;

		Handles& handles_;
		JobQueue& reserve_job_que_;
		JobQueue wait_job_que_;
		SyncMutexNode mutex_node_;
		SyncEventNode event_node_;
	};

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

	JobQueue& reserve_job_que() { return reserve_job_que_; }
	JobQueue& wait_job_que() { return wait_job_que_; }

	_NODISCARD SyncMutexNode mutex_node() { return mutex_hub_.make_node(); }

	bool RegistJob(TypeIds tids, JobBaseHub job);

	HandleByType handle_by_type_;
	Handles handles_;

	JobQueue reserve_job_que_;
	JobQueue wait_job_que_;

	SyncMutexHub mutex_hub_;
	SyncEventHub event_hub_;
	TaskID distribute_task_id_;
	TaskIDs worker_task_ids_;
};

template<typename ..._Tys>
bool SyncStation::RegistJob(JOB_TYPE type, JobBaseHub job)
{
	auto tids = TypeHarvest::GetInstance().harvest<_Tys...>();
	SingleLock single_lock(mutex_hub_);

	reserve_job_que_.emplace(type, tids, job);
	
	return true;
}