#pragma once
#include "stdafx.h"
#include "singleton.h"
#include "Object.h"

class BlockController final : public Singleton<BlockController>
{
private:	
	using Clasp = std::mutex;
	using ClaspType = WORD;
	using ClaspByType = std::map<ClaspType, Clasp>;
	using LockMaterial = std::unordered_map<TypeID, ClaspByType>;

public:

private:

	template<typename _Ty>
	bool RegistClasp()
	{

	}


	Clasp gate_keeper_;
};