#pragma once
#include "stdafx.h"
#include "Object.h"
#include "BlockContorller.h"

enum class LOCK_TYPE
{
	SHARED,
	EXCLUSIVE,
};

template<typename _Func>
using is_function = std::enable_if<std::is_function<_Func>::value, _Func>;

struct LockBase abstract
{
	
};

struct SharedLock : public LockBase
{
	std::shared_lock<SharedMtx> lock;
};

struct ExclusiveLock : public LockBase
{
	std::unique_lock<SharedMtx> lock;
};

template<typename _Func, typename is_function<_Func>::type * = nullptr>
class JobBase abstract
{
public:
	JobBase(LOCK_TYPE lock_type, _Func func)
		: func_(func)
	{}

protected:
	template<typename _Ty, typename ..._Tys>
	void ObtainLock(_Ty type, _Tys ...Args)
	{
		bind_try_lock_.emplace(BlockController::GetInstance().ObtainShrMtx(type));
		ObtainLock(Args...);
	}

	void ObtainLock()
	{
		
	}

	virtual bool RegistJob() abstract;
	virtual bool AsyncJob() abstract;
	virtual bool EndJob() abstract;

private:
	_Func func_;
	SharedMtxByType bind_mtx_by_type;
};