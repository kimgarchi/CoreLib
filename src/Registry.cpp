#include "stdafx.h"
#include "Registry.h"

Registry::~Registry()
{
	if (isOpened_)
	{
		close();
	}
}

bool Registry::open(HKEY rootKey, const RegisterString& subKey, const long option)
{
	if (isOpened_ != false)
		return false;

	if (RegOpenKeyEx(rootKey, subKey.c_str(), 0, option, &rootKey_) != ERROR_SUCCESS)
		return false;

	isOpened_ = true;

	return true;
}

bool Registry::close()
{
	if (isOpened_ == false)
		return false;

	if (RegCloseKey(rootKey_) != ERROR_SUCCESS)
		return false;

	isOpened_ = false;

	return true;
}

bool Registry::createKey(HKEY rootKey, const RegisterString& subKey, const long option)
{
	if (isOpened_ != false)
		return false;

	auto ret = RegCreateKeyEx(rootKey, subKey.c_str(), 0, NULL, option, KEY_CREATE_SUB_KEY, NULL, &rootKey_, NULL);
	if (ret != ERROR_SUCCESS)
		return false;

	isOpened_ = true;

	return true;
}

bool Registry::deleteKey(HKEY rootKey, const RegisterString& subKey, const long option)
{
	if (RegDeleteKeyEx(rootKey, subKey.c_str(), option, 0) != ERROR_SUCCESS)
		return false;

	return true;
}

bool Registry::setValue(const RegisterString& valueName, const DWORD value, const long option)
{
	if (isOpened_ == false)
		return false;

	if (RegSetValueEx(rootKey_, valueName.c_str(), 0, option, (LPBYTE)&value, sizeof(DWORD)) != ERROR_SUCCESS)
		return false;

	return true;
}

bool Registry::setValue(const RegisterString& valueName, const RegisterString& value, const long option)
{
	if (isOpened_ == false)
		return false;

	if (RegSetValueEx(rootKey_, valueName.c_str(), 0, option, (LPBYTE)value.c_str(), (DWORD)sizeof(RegisterString::value_type) * (DWORD)value.length()) != ERROR_SUCCESS)
		return false;

	return true;
}

bool Registry::getValue(const RegisterString& valueNmae, RegisterString& value)
{
	if (isOpened_ == false)
		return false;

	DWORD valueType = 0;
	RegisterString buffer;
	DWORD buffer_length = 0;

#ifdef UNICODE
	LPCWSTR p_value = value.c_str();
#else
	LPCTSTR p_value = value.c_str();
#endif
	auto ret = RegQueryValueEx(rootKey_, valueNmae.c_str(), 0, &valueType, (LPBYTE)p_value, &buffer_length);
	if (ret != ERROR_SUCCESS)
	{
		if (ret == ERROR_MORE_DATA)
		{
			value.resize(buffer_length);
			p_value = value.c_str();
			ret = RegQueryValueEx(rootKey_, valueNmae.c_str(), 0, &valueType, (LPBYTE)p_value, &buffer_length);
			if (ret != ERROR_SUCCESS)
				return false;
		}
		else
			return false;
	}

	return true;
}

bool Registry::getValue(const RegisterString& valueNmae, DWORD& value)
{
	if (isOpened_ == false)
		return false;

	DWORD valueType = 0;
	DWORD bufferLength = sizeof(DWORD);
	LPDWORD p_value = &value;

	if (RegQueryValueEx(rootKey_, valueNmae.c_str(), 0, &valueType, (BYTE*)p_value, &bufferLength) != ERROR_SUCCESS)
		return false;

	return true;
}

bool Registry::deleteValue(const RegisterString& valueName)
{
	if (isOpened_ == false)
		return false;

#ifdef UNICODE
	LPCWSTR p_valueName = valueName.c_str();
#else
	LPCTSTR p_valueName = valueName.c_str();
#endif
	if (RegDeleteValue(rootKey_, p_valueName) != ERROR_SUCCESS)
		return false;

	return true;
}