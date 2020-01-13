#pragma once
#include "stdafx.h"

#define ASSERT(cond, msg) if (!(cond)) { /* MessageBox(nullptr, msg, __FUNCTIONW__, MB_OK | MB_ICONERROR); */ assert(false); }
#define SAFE_DELETE(data) delete data; data = nullptr;

#define NONE 0b0000
#define MEM_READ 0b0001
#define MEM_WRITE 0b0010
#define ASYNC_IO 0b0100
#define MASK 0b1000