#pragma once
#include "stdafx.h"
#include "Object.h"
#include "SyncStation.h"

class JobBase abstract : public object
{
public:
	bool Run() { return Execute(); }

protected:
	virtual bool Execute() abstract;

private:
	friend class SyncStation;
};

using JobUnit = wrapper_node<JobBase>;

template<typename _Func>
class ReadJob
{
private:
	virtual bool Execute()
	{
		// Require Job Station prev
		//SyncStation::GetInstance().RegistReadJob();
		return true;
	}
};

template<typename _Func>
class WriteJob
{
public:

private:
};