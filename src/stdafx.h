// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS


// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.

#include <Windows.h>
#include <assert.h>

#include <Pdh.h>
#pragma comment(lib, "Pdh.lib")

#include <memory>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <fstream>
#include <iostream>
#include <sstream>
#include <condition_variable>
#include <functional>
#include <future>
#include <thread>
#include <cstdarg>
#include <typeindex>
#include <optional>

#include "MemoryAllocator.h"
#include "NonCopyableBase.h"