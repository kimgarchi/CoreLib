#pragma once
#include "stdafx.h"
#include "SyncStation.h"
#include "ThreadManager.h"

SyncStation::SyncStation()
	: mutex_hub_(make_wrapper_hub<SyncMutex>()), event_hub_(make_wrapper_hub<SyncEvent>()), distribute_task_id_(INVALID_ALLOC_ID)
{
	const size_t single_count = 1;
	distribute_task_id_ = ThreadManager::GetInstance().AttachTask<DistributeJob>(single_count, std::ref(handles_), std::ref(reserve_job_que_), mutex_hub_.make_node(), event_hub_.make_node());
	if (distribute_task_id_ == INVALID_ALLOC_ID)
	{
		assert(false);
		throw std::runtime_error("sync station thread bind failed");
	}
}

SyncStation::~SyncStation()
{
	assert(ThreadManager::GetInstance().DeattachTask(distribute_task_id_));
}

bool SyncStation::RecordHandle(TypeID tid, LONG read_job_max_count)
{
	if (handle_by_type_.find(tid) != handle_by_type_.end())
	{
		assert(false);
		return true;
	}
	
	auto array_idx = handle_by_type_.size() + 1;
	auto ret = handle_by_type_.emplace(tid, make_wrapper_hub<SyncSemaphore>(read_job_max_count));
	
	if (ret.second == false)
	{
		assert(false);
		return false;
	}

	handles_.emplace_back(ret.first->second->handle());

	return true;
}

SYNC_STATE SyncStation::handle_state(TypeID tid)
{
	std::unique_lock<std::mutex>(distribute_mtx_);

	auto itor = handle_by_type_.find(tid);
	if (itor == handle_by_type_.end())
	{
		assert(false);
		return SYNC_STATE::FULL_LOCK;
	}

	return itor->second->state();
}

bool SyncStation::IsRecordType(TypeID tid)
{
	auto itor = handle_by_type_.find(tid);
	return false;
}

bool SyncStation::RegistJob(TypeIds tids, JobBaseHub job)
{
	return false;
}

SyncStation::DistributeJob::DistributeJob(Handles& handles, JobQueue& reserve_job_que, SyncMutexNode mutex_node, SyncEventNode event_node)
	: handles_(handles), reserve_job_que_(reserve_job_que), mutex_node_(mutex_node), event_node_(event_node)
{
}

bool SyncStation::DistributeJob::Work()
{
	SingleLock single_lock(mutex_node_);

	std::vector<bool> signal_check(handles_.size(), { false, });
	auto ret = WaitForMultipleObjects(static_cast<DWORD>(handles_.size()), handles_.data(), false, INFINITE);
	//... check multiple handle signal state
	


	auto shift_size = ShiftQue();
	for (size_t i = 0; i < wait_job_que_.size(); ++i)
	{
		/*
		wait job que add new typeids
		and vector is realloc
		*/
		
	}


	return true;
}

size_t SyncStation::DistributeJob::ShiftQue()
{
	auto& reserve_que = reserve_job_que_;
	auto shift_size = reserve_que.size();

	while (reserve_que.empty() == false)
	{
		wait_job_que_.emplace_back(reserve_que.front());
		reserve_que.pop_front();
	}

	return shift_size;
}
