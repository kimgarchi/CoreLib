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
	class RWHandle;
	class ReservePackage;
	class DistributeJob;
	
	DEFINE_WRAPPER_HUB(RWHandle);
	DEFINE_WRAPPER_NODE(RWHandle);

	DEFINE_WRAPPER_HUB(ReservePackage);
	DEFINE_WRAPPER_NODE(ReservePackage);

	using HandleByType = std::unordered_map<TypeID, RWHandleHub>;
	using RWHandleNodes = std::vector<RWHandleNode>;
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

	class RWHandle : public object
	{
	public:
		RWHandle(LONG read_job_max_count);
		decltype(auto) state();
		inline decltype(auto) WriteHandle() { return mutex_.get(); }
		inline decltype(auto) Readhandle() { return semaphore_.get(); }

	private:
		SyncMutexHub mutex_;
		SyncSemaphoreHub semaphore_;
	};

	class ReservePackage : public object
	{
	public:
		ReservePackage(JOB_TYPE&& type, RWHandleNodes&& rw_handle_nodes, DisposableJobHub&& job_hub);
		virtual ~ReservePackage();

		bool Aquire();

		inline bool operator <(const ReservePackage& rhs) { return this->try_count_ < rhs.try_count_; }
		inline bool operator >(const ReservePackage& rhs) { return this->try_count_ > rhs.try_count_; }

	private:
		friend class SyncStation;
		friend class DistributeJob;

		void increase_try_count() { try_count_ += 1; }

		JOB_TYPE type_;
		RWHandleNodes rw_handle_nodes_;
		Handles read_handles_;
		Handles write_handles_;
		DisposableJobHub job_hub_;
		ULONGLONG try_count_;
	};

	class DistributeJob : public JobBase
	{
	public:
		DistributeJob(HandleByType& handle_by_type, ReserveJobQue& reserve_job_que, SyncMutexNode mutex_node, SyncEventNode event_node);
		virtual bool Work() override;

	private:
		using HandleWait = std::vector<bool>;

		size_t ShiftQue();

		HandleWait handle_wait_;

		HandleByType& handle_by_type_;
		ReserveJobQue& reserve_job_que_;
		WaitJobPriorityQueue wait_job_priority_que_;
		SyncMutexNode mutex_node_;
		SyncEventNode event_node_;
	};

	bool RecordHandle(TypeID tid, LONG read_job_max_count);
	SYNC_STATE handle_state(TypeID tid);
	bool IsRecordType(TypeID tid);

	_NODISCARD SyncMutexNode mutex_node() { return mutex_hub_.make_node(); }

	bool RegistJob(TypeIds tids, JobBaseHub job);

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
	auto tids = TypeHarvest::GetInstance().harvest<_Tys...>();
	SingleLock single_lock(mutex_hub_);

	reserve_job_que_.emplace(make_wrapper_hub<ReservePackage>(type, tids, job));
	
	return true;
}