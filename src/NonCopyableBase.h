#pragma once

class NonCopyableBase abstract
{
public:
	NonCopyableBase() = default;
	virtual ~NonCopyableBase() = default;

	NonCopyableBase(const NonCopyableBase&) = delete;
	NonCopyableBase& operator=(const NonCopyableBase&) = delete;
	NonCopyableBase(NonCopyableBase&&) = delete;
	NonCopyableBase& operator=(NonCopyableBase&&) = delete;
};