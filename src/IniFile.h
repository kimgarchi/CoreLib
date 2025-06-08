#pragma once
#include "stdafx.h"
#include <tchar.h>

class IniFile final
{
public:
#ifdef UNICODE
	using IniString = std::wstring;
#else
	using IniString = std::string;
#endif

	IniFile(const IniString& file_name) : file_name_{ file_name } {}
	~IniFile() = default;

	template<typename _Ty>
	bool setValue(const IniString& key_name, const IniString& value_name, const _Ty& value);

	template<typename _Ty>
	std::optional<_Ty> getValue(const IniString& key_name, const IniString& value_name, const _Ty default_value);

private:
	const IniString file_name_;
};

template<typename T>
struct is_wchar_array : std::false_type {};

template<std::size_t N>
struct is_wchar_array<const wchar_t[N]> : std::true_type {};

template<typename T>
constexpr bool is_wchar_array_v = is_wchar_array<std::remove_cvref_t<T>>::value;

template<typename _Ty>
inline bool IniFile::setValue(const IniString& key_name, const IniString& value_name, const _Ty& value)
{
	IniString val_str{ value };
	if (val_str.empty())
	{
		return false;
	}

	return WritePrivateProfileString(key_name.c_str(), value_name.c_str(), val_str.c_str(), file_name_.c_str());
}

template<typename _Ty>
inline std::optional<_Ty> IniFile::getValue(const IniString& key_name, const IniString& value_name, _Ty default_value)
{
	if constexpr (std::is_integral_v<_Ty>)
	{
		_Ty value = GetPrivateProfileInt(key_name.c_str(), value_name.c_str(), default_value, file_name_.c_str());
		if (value == default_value)
			return std::nullopt;

		return { value };
	}
	else if constexpr (std::is_floating_point_v<_Ty>)
	{
		IniString value;
		const DWORD length{ 256 };
		value.resize(length);

		DWORD ret = 0;
#ifdef UNICODE
		LPWSTR p_value = value.c_str();
#else
		PSTR p_value = value.c_str();
#endif

		ret = GetPrivateProfileString(key_name.c_str(), value_name.c_str(), default_value, p_value, length, file_name_.c_str());

		if (ret == -1)
			return std::nullopt;
#ifdef UNICODE
		return { std::wcstof(value) };
#else
		return { std::stof(value) };
#endif
	}
	else
	{
		IniString value;
		const DWORD length{ 256 };
		value.resize(length);

		DWORD ret = 0;
#ifdef UNICODE
		LPWSTR p_value = const_cast<LPWSTR>(value.c_str());
		LPCWSTR p_default_value = const_cast<LPCWSTR>(default_value.c_str());
#else
		LPTSTR p_value = const_cast<LPTSTR>(value.c_str());
		LPCTSTR p_default_value = const_cast<LPCTSTR>(default_value.c_str());
#endif

		ret = GetPrivateProfileString(key_name.c_str(), value_name.c_str(), p_default_value, p_value, length, file_name_.c_str());
		if (ret == -1)
			return std::nullopt;

		return { value };
	}
}
