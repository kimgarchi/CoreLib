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
	class ReservePackage;
	class DistributeJob;
	
	DEFINE_WRAPPER_HUB(ReservePackage);
	DEFINE_WRAPPER_NODE(ReservePackage);

	using HandleByType = std::unordered_map<TypeID, SyncSemaphoreHub>;
	using Handles = std::vector<HANDLE>;
	using ReserveJobQue = std::queue<ReservePackageHub>;
	using WaitJobPriorityQueue = std::priority_queue<ReservePackageHub, std::vector<ReservePackageHub>, std::less<ReservePackageHub>>;

public:
	SyncStation();
	virtual ~SyncStation();

	template<typename ... _Tys>
	bool RegistJob(JOB_TYPE type, JobBaseHub job);

private:
	friend class DistributeJob;

	class ReservePackage : public object
	{
	public:
		ReservePackage(JOB_TYPE&& type, TypeIds&& tids, JobBaseNode&& job_hub);
		virtual ~ReservePackage();

		bool Aquire();
		virtual const size_t priority_value() const override { return try_count_; }

	private:
		friend class SyncStation;
		friend class DistributeJob;

		inline void increase_try_count() { ++try_count_; }
		
		const JOB_TYPE type_;
		TypeIds tids_;
		Handles handles_;
		JobBaseNode job_node_;
		ULONGLONG try_count_;
	};

	class DistributeJob : public JobBase
	{
	public:
		DistributeJob(HandleByType& handle_by_type, ReserveJobQue& reserve_job_que, SyncMutexNode mutex_node, SyncEventNode event_node);
		virtual bool Work() override;

	private:
		bool Prepare(ReservePackageHub& package __inout);

		HandleByType& handle_by_type_;
		ReserveJobQue& reserve_job_que_;
		WaitJobPriorityQueue wait_job_priority_que_;
		SyncMutexNode mutex_node_;
		SyncEventNode event_node_;
	};

	HANDLE RecordHandle(TypeID tid, LONG read_job_max_count = _default_read_count_);
	bool IsRecordType(TypeID tid) { return (handle_by_type_.find(tid) != handle_by_type_.end()); }

	HandleByType handle_by_type_;
	ReserveJobQue reserve_job_que_;
	SyncMutexHub mutex_hub_;
	SyncEventHub event_hub_;
	TaskID distribute_task_id_;
	TaskIDs worker_task_ids_;
};

template<typename ..._Tys>
bool SyncStation::RegistJob(JOB_TYPE type, JobBaseHub job)
{
	TypeIds tids = TypeHarvest::GetInstance().harvest<_Tys...>();
	SingleLock single_lock(mutex_hub_);

	reserve_job_que_.emplace(make_wrapper_hub<ReservePackage>(type, tids, job.make_node()));
	assert(event_hub_->Release());

	return true;
}