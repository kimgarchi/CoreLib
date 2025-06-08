#pragma once

class Registry final
{
public:
#ifdef UNICODE
	using RegisterString = std::wstring;
#else
	using RegisterString = std::string;
#endif

	Registry() = default;
	~Registry();

	bool open(HKEY rootKey, const RegisterString& subKey, const long option = 0L);
	bool close();

	bool createKey(HKEY rootKey, const RegisterString& subKey, const long option = REG_OPTION_NON_VOLATILE);
	bool deleteKey(HKEY rootKey, const RegisterString& subKey, const long option = 0L);

	bool setValue(const RegisterString& valueName, const DWORD value, const long option = REG_DWORD);
	bool setValue(const RegisterString& valueName, const RegisterString& value, const long option = REG_SZ);

	bool getValue(const RegisterString& valueNmae, RegisterString& value __out);
	bool getValue(const RegisterString& valueNmae, DWORD& value __out);

	bool deleteValue(const RegisterString& valueName);

private:
	HKEY rootKey_{ nullptr };
	bool isOpened_{ false };
};

