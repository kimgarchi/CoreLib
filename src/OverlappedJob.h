#pragma once
#include "stdafx.h"
#include "Job.h"

class OverlappedJob : public JobBase
{
public:
	OverlappedJob(JobBaseNode&& job_node);
	virtual ~OverlappedJob();

protected:
	virtual bool commit() abstract;
	virtual bool rollback() abstract;

private:
	virtual bool Work() override;

	JobBaseNode job_node_;
	OVERLAPPED overlapped_;
};

DEFINE_WRAPPER_HUB(OverlappedJob);
DEFINE_WRAPPER_NODE(OverlappedJob);