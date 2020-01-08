#pragma once
#include "stdafx.h"

#define BUFFER_MAX_SIZE 1024

struct DataStream
{
public:
	DataStream()
		: buffer_position(0)
	{
		//memset(buffer, 0, BUFFER_MAX_SIZE);
	}

	template<typename T>
	void operator << (const T& data)
	{
		WriteData(data);
	}

	void operator << (const char* data)
	{
		BYTE size = sizeof(data);
		for (BYTE i = 0; i < size; ++i)
			WriteData(data[i]);

		WriteData(size);
	}

	void operator << (const std::wstring& data)
	{
		BYTE size = static_cast<BYTE>(data.length());
		for (BYTE i = 0; i < size; ++i)
			WriteData(data[i]);
		
		WriteData(size);
	}

	void operator << (const std::string& data)
	{
		BYTE size = static_cast<BYTE>(data.length());
		for (BYTE i = 0; i < size; ++i)
			WriteData(data[i]);

		WriteData(size);
	}

	void operator << (const float& data)
	{
		
	}

	void operator << (const double& data)
	{

	}

	template<typename T>
	void operator >> (T& data)
	{
		ReadData(data);
	}
	/*
	void operator >> (const wchar_t* data)
	{
		BYTE size = 0;
		ReadData(size);

		for (BYTE i = size; i > 0;)
		{
			--i;

			ReadData(data[i]);
		}
	}
	*/
	void operator >> (std::string& data)
	{
		BYTE size = 0;
		ReadData(size);

		data.resize(size);

		for (BYTE i = size; i > 0;)
		{
			--i;

			ReadData(data[i]);
		}
	}
	
	void operator >> (std::wstring& data)
	{
		BYTE size = 0;
		ReadData(size);

		data.resize(size);

		for (BYTE i = size; i > 0;)
		{
			--i;

			ReadData(data[i]);
		}
	}

private:

	template<typename T>
	void ReadData(T& data)
	{
		assert(buffer_position - sizeof(data) >= 0);

		buffer_position -= sizeof(data);
		data = buffer[buffer_position];		
	}

	template<typename T>
	void WriteData(const T& data)
	{
		assert(buffer_position + sizeof(data) <= BUFFER_MAX_SIZE);

		buffer[buffer_position] = data;
		buffer_position += sizeof(data);
	}


	BYTE buffer_position;
	BYTE buffer[BUFFER_MAX_SIZE];
};