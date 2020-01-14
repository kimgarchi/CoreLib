#pragma once
#include "stdafx.h"
#include "Object.h"
#include "BlockContorller.h"

template<typename _Func, typename ..._Funcs>
class JobBase abstract : public object
{
private:
	using LockTypes = std::set<TypeID>;

public:
	JobBase(_Func func)
		: func_(func)
	{}

	virtual ~JobBase()
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

	template<typename ..._Tys>
	decltype(auto) Run(_Tys... Args) { return func_(Args...); }

	virtual bool RegistAsyncJob() abstract;


private:
	bool RecordLockType(TypeID tid) { return lock_types_.emplace(tid).second; }

	_Func func_;
	LockTypes lock_types_;
};

template<typename _Func, typename ..._Funcs>
class ReadJob
{
public:

private:
};

template<typename _Func, typename ..._Funcs>
class WriteJob
{
public:

private:
};