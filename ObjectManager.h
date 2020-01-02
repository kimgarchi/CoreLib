#pragma once
#include "stdafx.h"
#include "Object.h"
#include "singleton.h"

class object;
class ObjectManager final : public Singleton<ObjectManager>
{
private:
	struct UsingData
	{
		UsingData(ULONGLONG cur_tick = GetTickCount64())
			: alloc_tick(cur_tick), last_use_tick(cur_tick)
		{}

		ULONGLONG alloc_tick;
		ULONGLONG last_use_tick;
	};

	using Trace = std::map<object*, UsingData>;
	using TraceByAllocId = std::unordered_map<AllocID, Trace>;
	using TraceObjects = std::unordered_map<TypeID, TraceByAllocId>;
	

public:
	ObjectManager() {}
	ObjectManager(const ObjectManager&) = delete;
	ObjectManager& operator= (const ObjectManager&) = delete;

	~ObjectManager() 
	{
	}
	
	template<typename _Ty>
	void Regist(_Ty* object)
	{
		auto tid = typeid(_Ty).hash_code();
		auto type_itor = trace_objects_.find(tid);
		if (type_itor == trace_objects_.end())
		{
		}
	}

	template<typename _Ty>
	void UnRegist(_Ty* object)
	{
		auto tid = typeid(_Ty).hash_code();
		auto type_itor = trace_objects_.find(tid);
		if (type_itor == trace_objects_.end())
			assert(false);
		

	}

private:

	


	TraceObjects trace_objects_;
	const static ULONGLONG trace_unused_tick_ = 30000;
};
