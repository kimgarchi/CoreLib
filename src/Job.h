#pragma once
#include "stdafx.h"
#include "Object.h"
#include "BlockContorller.h"

enum class LOCK_TYPE
{
	EXCLUSIVE,
	SHARED,	
};

template<typename _Func>
using is_function = typename std::enable_if_t<std::is_function_v<_Func>, _Func>;

template<typename _Func, is_function<_Func> = nullptr>
class JobBase abstract
{
public:
	JobBase()
		: func_(func)
	{}

protected:

	template <typename _Ty>
	void RecordType()
	{
		types.emplace_back(std::string(typeid(_Ty).name()));

		std::for_each(types.begin(), types.end(),
			[](std::string str)
		{
			std::cout << str << std::endl;
		});

		func_(12, 13);
	}

	template<typename _Ty, typename ..._Tys>
	void RecordType()
	{
		RecordType<_Ty, _Tys...>();
	}

	template<typename _Ty, typename ..._Tys>
	void RecordLockType(_Ty type, _Tys ...Args)
	{
		bind_try_lock_.emplace(BlockController::GetInstance().ObtainShrMtx(type));
		ObtainLock(Args...);
	}

	void ObtainLock()
	{
		
	}

	virtual bool RegistJob() abstract;
	virtual bool AsyncJob() abstract;
	virtual bool SyncJob() abstract;

private:

	_Func func_;
};