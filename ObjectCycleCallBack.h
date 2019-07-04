#pragma once
#include "CallBack.h"
#include "Object.h"

class ObjectCycleCallBack : public CallBack
{
public:
	ObjectCycleCallBack(std::shared_ptr<Object> object, ULONGLONG notify_waring_process_tick = 2000);
	virtual ~ObjectCycleCallBack();

	virtual bool IsCall() override;
	virtual bool ProcessJob() override;


private:
	std::shared_ptr<Object> object_;
};

ObjectCycleCallBack::ObjectCycleCallBack(std::shared_ptr<Object> object, ULONGLONG notify_waring_process_tick)
	: CallBack(notify_waring_process_tick), object_(object)
{
}

ObjectCycleCallBack::~ObjectCycleCallBack()
{
}

bool ObjectCycleCallBack::IsCall()
{
}

bool ObjectCycleCallBack::ProcessJob()
{
}