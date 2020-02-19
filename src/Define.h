#pragma once
#include "stdafx.h"

#define ASSERT(cond, msg) if (!(cond)) { /* MessageBox(nullptr, msg, __FUNCTIONW__, MB_OK | MB_ICONERROR); */ assert(false); }
#define SAFE_DELETE(data) delete data; data = nullptr;

#define INVALID_JOB_ID 0

using ThrFunc = std::function<void()>;
using CondVar = std::condition_variable;

#define DEFINE_WRAPPER_HUB(Type) using Type##Hub = wrapper_hub<Type>;
#define DEFINE_WRAPPER_NODE(Type) using Type##Node = wrapper_node<Type>;