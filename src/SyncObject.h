#pragma once
#include "stdafx.h"

class SyncObject abstract
{
public:
	SyncObject(HANDLE handle);
	SyncObject& operator=(const SyncObject&) = delete;
	virtual ~SyncObject() = default;

protected:
	HANDLE handle_;
};

class SyncLock abstract : public SyncObject
{
public:	
	SyncLock(HANDLE handle);
	virtual ~SyncLock();

	virtual DWORD lock(ULONG cnt, DWORD timeout) abstract;
	virtual DWORD unlock(ULONG cnt) abstract;
	virtual ULONG lock_count() abstract;
	virtual ULONG max_lock_count() const abstract;

	bool try_lock(ULONG cnt);
	bool is_lock(ULONG cnt);
};

class SyncMutex : public SyncLock
{
public:
	SyncMutex(BOOL b_init = FALSE);
	virtual ~SyncMutex();

	virtual DWORD lock(ULONG cnt = 1, DWORD timeout = INFINITE) override;
	virtual DWORD unlock(ULONG cnt = 1) override;	
	virtual ULONG lock_count() override;
	virtual ULONG max_lock_count() const override;
};

class SyncSemaphore : public SyncLock
{
public:
	SyncSemaphore(LONG max_cnt);
	SyncSemaphore(LONG init_cnt, LONG max_cnt);
	virtual ~SyncSemaphore();

	virtual DWORD lock(ULONG cnt = 1, DWORD timeout = INFINITE) override;
	virtual DWORD unlock(ULONG cnt = 1) override;
	
	virtual ULONG lock_count() override;
	virtual ULONG max_lock_count() const override;

private:
	const ULONG init_lock_cnt_;
	const ULONG max_lock_cnt_;
};

class SyncEvent : public SyncObject
{
public:
	SyncEvent(BOOL is_menual_reset = false, BOOL init_state = false, const std::shared_ptr<SECURITY_ATTRIBUTES> security_attributes = nullptr, std::wstring name = L"");
	virtual ~SyncEvent();
	
	DWORD wait_signaled(DWORD timeout = INFINITE);
	DWORD raise_signaled();

	inline BOOL is_menual_reset() const { return is_menual_reset_; }
	inline BOOL init_state() const { return init_state_; }
	
private:
	const std::shared_ptr<SECURITY_ATTRIBUTES> security_attributes_;
	const BOOL is_menual_reset_;
	const BOOL init_state_;
};