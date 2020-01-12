#pragma once
#include "stdafx.h"
#include "singleton.h"
#include "SyncObject.h"

class SyncStation : public Singleton<SyncStation>
{
private:
	using SyncObjects = std::map<TypeID, SyncHub>;
	using Lock = std::unique_lock<std::mutex>;
public:	
	
	template<typename _Ty>
	bool ObtainSyncObjects()
	{
		Lock(inner_mtx_);

		auto tid = typeid(_Ty).hash_code();
		sync_objects_.emplace(tid, make_wrapper_hub<Mutex>());
	}

private:
	SyncObjects sync_objects_;
	std::mutex inner_mtx_;
};