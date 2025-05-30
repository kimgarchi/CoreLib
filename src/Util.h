#pragma once
#include "stdafx.h"

#pragma warning (push)
#pragma warning (disable : 26444)

using TypeIds = std::set<size_t>;
class TypeHarvest
{
public:
	template<typename ..._Tys>
	inline decltype(auto) harvest() 
	{
		return harvect_type<_Tys...>();
	}

private:
	template <typename _Ty>
	void harvect_type(TypeIds& tids) { tids.emplace(typeid(_Ty).hash_code()); }

	template<typename _fTy, typename _sTy, typename ..._Tys>
	void harvect_type(TypeIds& tids)
	{
		tids.emplace(typeid(_fTy).hash_code());
		harvect_type<_sTy, _Tys...>(tids);
	}

	template<typename _fTy, typename _sTy, typename ..._Tys>
	decltype(auto) harvect_type()
	{
		TypeIds tids{ typeid(_fTy).hash_code() };
		harvect_type<_sTy, _Tys...>(tids);
		return tids;
	}
};
/*
class CPUsage
{
private:
	using Specimens = std::queue<float>;
	using SpecimenCount = size_t;
	using TotalSpecimentUsage = float;

public:
	CPUsage(SpecimenCount count = 10)
	{
		PdhOpenQuery(NULL, NULL, &cpuQuery);
		PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
		PdhCollectQueryData(cpuQuery);
	}

	decltype(auto) Get()
	{
		PdhCollectQueryData(cpuQuery);
		PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
		return counterVal;
	}

private:
	PDH_HQUERY cpuQuery;
	PDH_HCOUNTER cpuTotal;
	PDH_FMT_COUNTERVALUE counterVal;

};
*/
#pragma warning (pop)