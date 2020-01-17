#pragma once
#include "stdafx.h"
#include "singleton.h"

#pragma warning (push)
#pragma warning (disable : 26444)

class TypeHarvest : public Singleton<TypeHarvest>
{
protected:
	using HarvestTypes = std::vector<size_t>;
	using Types = std::set<size_t>;

public:
	template<typename ..._Tys>
	decltype(auto) harvest() 
	{
		Types tids = harvect_type<_Tys...>();
		HarvestTypes havest_types;
		std::copy(tids.begin(), tids.end(), std::back_inserter(havest_types));
		return havest_types;
	}

private:
	template <typename _Ty>
	void harvect_type(Types& tids) { tids.emplace(typeid(_Ty).hash_code()); }

	template<typename _fTy, typename _sTy, typename ..._Tys>
	void harvect_type(Types& tids)
	{
		tids.emplace(typeid(_fTy).hash_code());
		harvect_type<_sTy, _Tys...>(tids);
	}

	template<typename _fTy, typename _sTy, typename ..._Tys>
	decltype(auto) harvect_type()
	{
		Types tids{ typeid(_fTy).hash_code() };
		harvect_type<_sTy, _Tys...>(tids);
		return tids;
	}
};

#pragma warning (pop)