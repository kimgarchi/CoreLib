#pragma once
#include "stdafx.h"
#include "singleton.h"
#include "Object.h"

using SharedMtx = std::shared_mutex;
using SharedMtxByType = std::map<TypeID, SharedMtx>;

class BlockController final : public Singleton<BlockController>
{
private:	
	

public:

private:
	template<typename _Ty>
	SharedMtx ObtainShrMtx()
	{
		std::shared_mutex mtx;
		TypeID tid = typeid(_Ty).hash_code();
		auto itor = shared_mtx_by_type_.find(tid);
		if (itor == shared_mtx_by_type_.end())
			shared_mtx_by_type_.emplace(tid, mtx);
		else
			mtx = itor->second;

		return mtx;
	}

	SharedMtxByType shared_mtx_by_type_;
};