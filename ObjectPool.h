#pragma once
#include "stdafx.h"
#include "Object.h"

enum class OBJECT_STATUS
{
	OBJECT_STATUS_UNKNOWN,
	OBJECT_STATUS_IDLE,
	OBJECT_STATUS_INACTIVE_WAIT,
	OBJECT_STATUS_USED,

	OBJECT_STATUS_MAX,
};

class ObjectPool
{
public:
	ObjectPool(DWORD min_object_count, DWORD max_object_count, DWORD variance_object_count);
	virtual ~ObjectPool();

	ObjectPool() = delete;
	ObjectPool(const ObjectPool&) = delete;
	void operator=(const ObjectPool&) = delete;

	virtual bool Initialize();
	OBJECT_STATUS GetObjectStatus(std::shared_ptr<Object> object);
	
	std::shared_ptr<Object> Get();
	bool Return(std::shared_ptr<Object> object);
	
	DWORD GetIdleChunkSize() { return static_cast<DWORD>(idle_objects_.size()); }
	DWORD GetUseChunkSize() { return static_cast<DWORD>(used_objects_.size()); }
	DWORD GetTotalChunkSize() { return static_cast<DWORD>(GetIdleChunkSize() + GetUseChunkSize()); }
	
protected:	
	virtual std::shared_ptr<Object> CreateObject() = 0;

private:
	using ChunkStatusMap = std::map<std::shared_ptr<Object>, OBJECT_STATUS>;
	using ChunkTickMap = std::map<std::shared_ptr<Object>, ULONGLONG>;
	using ChunkSet = std::set< std::shared_ptr<Object>>;
	
	friend class Object;

	bool IncreaseObjectCount();
	bool DecreaseObjectCount();
	bool ChangeObjectStatus(std::shared_ptr<Object> object, OBJECT_STATUS status);

	const static BYTE idle_object_use_count_ = 2;

	ChunkSet idle_objects_;
	ChunkTickMap inactive_wait_objects_;
	ChunkSet used_objects_;

	ChunkStatusMap objects_;

	DWORD min_idle_object_count_;
	DWORD max_object_count_;
	DWORD variance_object_count_;

	std::mutex mtx_;
};