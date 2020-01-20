#pragma once
#include "stdafx.h"
#include "Object.h"

__interface ThreadBase
{
public:
	virtual bool Run() abstract;
	virtual bool Stop() abstract;
	virtual bool Join() abstract;
};


class Thread : public ThreadBase, public object
{
private:

public:

private:
	std::thread thread_;
};