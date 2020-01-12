#pragma once
#include "stdafx.h"
#include "Object.h"
#include "BlockContorller.h"

template<typename _ReturnType, typename ..._Tys>
class Function
{
private:
	using Func = std::function<_ReturnType(_Tys... Args)>;

public:
	Function(Func func) : func_(func) {}
	_ReturnType operator ()(_Tys... Args) { return func_(std::forward<_Tys>(Args)...); }

private:
	const Func func_;
};

class JobBase abstract
{
private:
	using LockTypes = std::set<TypeID>;

public:
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

	LockTypes lock_types_;
};