#pragma once
#include "stdafx.h"
#include "singleton.h"
#include "Wrapper.h"

class BlockController final : public Singleton<BlockController>
{
private:
	using TypeCount = size_t;
	
public:
	BlockController()
	{	
	}

	bool BindWorker(DWORD worker_count)
	{

	}

private:

	

	std::mutex mtx_;
};