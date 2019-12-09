#pragma once
#include "stdafx.h"
#include "ObjectStation.h"

namespace
{
	class object abstract
	{
	public:
		virtual ~object();

		template<typename... _Tys>
		void initilize(_Tys&&... _Args) abstract;
	};

	object::~object()
	{
	}
}
