#pragma once
#include "stdafx.h"
#include "Object.h"
#include "BlockContorller.h"

template<typename _Func>
class JobBase
{
private:
	using LockTypes = std::set<TypeID>;

public:
	JobBase(_Func func)
		: func_(func)
	{}

	template <typename _Ty>
	void RecordLockType()
	{
		RecordLockType(typeid(_Ty).hash_code());
	}

	template<typename _fTy, typename _sTy, typename ..._Tys>
	void RecordLockType()
	{
		RecordLockType(typeid(_fTy).hash_code());
		RecordLockType<_sTy, _Tys...>();
	}

private:
	bool RecordLockType(TypeID tid) { return lock_types_.emplace(tid).second; }

	virtual bool AsyncJob() abstract;
	virtual bool SyncJob() abstract;
	
	_Func func_;
	LockTypes lock_types_;	
};

