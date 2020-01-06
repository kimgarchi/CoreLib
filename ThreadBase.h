#pragma once
#include "stdafx.h"
#include <thread>
#include "Object.h"

class ThreadBase abstract
{
private:
	using Thread = std::thread;

	enum class STATE
	{
		IDLE,
		ACTIVE,
	};

public:
	ThreadBase()
		: state_(STATE::IDLE), thread_()
	{}

	virtual bool run() abstract;
	virtual bool stop() abstract;

	virtual DWORD TryLock() abstract;
	virtual DWORD TryUnLock() abstract;

private:

	STATE state_;
	Thread thread_;
};