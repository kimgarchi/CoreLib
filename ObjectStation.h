#pragma once
#include "stdafx.h"
#include "ObjectPool.h"
#include "wrapper.h"

namespace
{
	class ObjectStation final
	{
	public:
		
	private:
		using TID = size_t;
		using ObjectPools = std::map<TID, PVOID>;

		static DWORD min_object_count_;
		static DWORD max_object_count_;
		static DWORD variance_object_count_;

		ObjectPools object_pools_;
	};

}