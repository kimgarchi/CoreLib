#pragma once
#include "stdafx.h"
#include "Object.h"


template<typename _Ty>
class ObjectPool
{
public:


private:
	using ChunkQue = std::queue<_Ty*>;
	using ChunkSet = std::set<_Ty*>;

	static bool Initialize(DWORD min_object_count, DWORD max_object_count, DWORD variance_object_count);		

	ObjectPool<_Ty>();
	~ObjectPool<_Ty>();

	ObjectPool<_Ty>(const ObjectPool<_Ty>&) = delete;
	void operator=(const ObjectPool<_Ty>&) = delete;

	bool Initialize();

	bool Push(_Ty* object);
	_Ty* Pop();

	DWORD GetIdleChunkSize() { return static_cast<DWORD>(chunk_que_.size()); }
	DWORD GetUseChunkSize() { return static_cast<DWORD>(abs(chunks_.size() - chunk_que_.size())); }
	DWORD GetTotalChunkSize() { return static_cast<DWORD>(chunks_.size()); }

	bool IncreasePool();
	bool DecreasePool();

	ChunkQue chunk_que_;
	ChunkSet chunks_;

	static DWORD min_idle_object_count_;
	static DWORD max_object_count_;
	static DWORD variance_object_count_;

	std::mutex mtx_;
};