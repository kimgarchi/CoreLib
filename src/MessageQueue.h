#pragma once
#include "SyncObject.h"

class SyncMutex;
class MessageQueue final
{
public:
	MessageQueue();
	~MessageQueue();

private:

	SyncMutex mutex_;
};

