#pragma once
#include "stdafx.h"

const static DWORD _default_job_timeout_ = 3000;

class JobBase abstract
{
public:
	void Execute()
	{
		if (Run() == true)
		{
			Commit();
		}
		else
		{
			Rollback();			
		}
	}

protected:
	virtual bool Run() abstract;
	virtual void Commit() abstract;
	virtual void Rollback() abstract;
};