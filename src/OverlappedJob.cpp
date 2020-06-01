#include "stdafx.h"
#include "OverlappedJob.h"
#include "SyncStation.h"

OverlappedJob::OverlappedJob(JobBaseNode&& job_node)
	: job_node_(job_node)
{
	memset(&overlapped_, 0x00, sizeof(OVERLAPPED));
}

OverlappedJob::~OverlappedJob()
{
	
}