#pragma once
#include "stdafx.h"

enum class CALLBACK_STATUS
{
	CALLBACK_STATUS_WAIT,
	CALLBACK_STATUS_COMPLETE,
};

class CallBack
{
public:
	CallBack(ULONGLONG notify_waring_process_tick);
	virtual ~CallBack();

	ULONGLONG GetEnteringTickCount() { return entering_tick_count_; }

	virtual bool IsCall() = 0;
	virtual bool ProcessJob() = 0;

private:
	friend class CallBackManager;

	bool IsCall(ULONGLONG update_tick_count);

	ULONGLONG notify_waring_process_tick_;
	ULONGLONG last_update_tick_count_;
	ULONGLONG entering_tick_count_;
};

CallBack::CallBack(ULONGLONG notify_waring_process_tick)
	: notify_waring_process_tick_(notify_waring_process_tick), last_update_tick_count_(0), entering_tick_count_(0)
{
}

CallBack::~CallBack()
{
}

bool CallBack::IsCall(ULONGLONG update_tick_count)
{
	if (update_tick_count - last_update_tick_count_ >= notify_waring_process_tick_)
	{
		// log...
		assert(false);
	}

	last_update_tick_count_ = update_tick_count;
	if (IsCall() == false)
		return false;

	return true;
}
