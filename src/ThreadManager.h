#pragma once
#include "singleton.h"
#include "Wrapper.h"
#include "Thread.h"

using ThreadHub = wrapper_hub<Thread>;
using ThreadNode = wrapper_node<Thread>;
using ChunkThread = std::list<Thread>;
using ThreadLine = std::unordered_map<TypeID, ChunkThread>;

class ThreadManager : Singleton<ThreadManager>
{
public:

private:
	ThreadLine thread_line_;
};