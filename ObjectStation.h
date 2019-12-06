#pragma once
#include "stdafx.h"
#include "ObjectPool.h"
#include "wrapper.h"

namespace co
{
	class ObjectStation final
	{
	public:
		ObjectStation();

		template<typename _Ty>
		bool RegistType();

	private:
		using TID = size_t;
		using ObjectPools = std::map<TID, PVOID>;

		static bool duplicated_create;

		ObjectPools object_pools_;
		
	};
}