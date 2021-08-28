#pragma once
#include "stdafx.h"

#define ASSERT(cond, msg) if (!(cond)) { /* MessageBox(nullptr, msg, __FUNCTIONW__, MB_OK | MB_ICONERROR); */ assert(false); }
#define SAFE_DELETE(data) delete data; data = nullptr;

#define INVALID_JOB_ID 0

using CondVar = std::condition_variable;
using TaskID = size_t;
using TaskIDs = std::vector<TaskID>;
using ThreadID = HANDLE;
using AllocID = size_t;
using TypeID = size_t;
using Count = std::atomic_size_t;


#define DEFINE_WRAPPER_HUB(Type) using Type##Hub = wrapper_hub<Type>;
#define DEFINE_WRAPPER_NODE(Type) using Type##Node = wrapper_node<Type>;

#define DEFINE_WRAPPER(Type)	\
	DEFINE_WRAPPER_HUB(Type)	\
	DEFINE_WRAPPER_NODE(Type)

#define WAIT_TIME_ZERO 0

#define SECONDS_TO_TICK 1000

#define MSG_BUF_SIZE 4096