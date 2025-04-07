#pragma once
#include "stdafx.h"
#include "LockObject.h"

class JobBase abstract
{
public:
	void AttachJobMutex(std::shared_ptr<SyncMutex> job_mutex) { job_mutex_ = job_mutex; }

	void Execute()
	{
		if (job_mutex_ == nullptr)
		{
			JobProgress();
		}
		else
		{
			SingleLock lock(*job_mutex_);
			JobProgress();
		}
	}

	void JobTerminate()
	{
		if (job_complete_)
		{
			///
			return;
		}
	
		if (job_mutex_ == nullptr)
		{
			Rollback();
		}
		else
		{
			SingleLock lock(*job_mutex_);
			Rollback();
		}
	}

protected:
	virtual bool Run() abstract;
	virtual void Commit() abstract;
	virtual void Rollback() abstract;
	
private:
	void JobProgress()
	{
		if (Run())
		{
			Commit();
		}
		else
		{
			Rollback();
		}

		job_complete_ = true;
	}

	bool job_complete_ = false;
	std::shared_ptr<SyncMutex> job_mutex_;
};