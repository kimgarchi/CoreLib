#pragma once
#include "stdafx.h"
#include <sql.h>

enum class COLUMN_TYPE_INTEGER
{
	NUMERIC = SQL_NUMERIC,
	INT = SQL_INTEGER,
	SMALLINT = SQL_SMALLINT,
	DATETIME = SQL_DATETIME,
};

enum class COLUMN_TYPE_DECIMAL
{
	DECIMAL = SQL_DECIMAL,
	REAL = SQL_REAL,
	FLOAT = SQL_FLOAT,
	DOUBLE = SQL_DOUBLE,
};

enum class COLUMN_TYPE_STRING
{
	CHAR = SQL_CHAR,
	VARCHAR = SQL_VARCHAR,
};

class Column abstract
{
protected:
	enum class COLUMN_TYPE;

public:
	Column(COLUMN_TYPE type, std::wstring name, bool is_null)
		: type_(type), name_(name), is_null_(is_null)
	{}

	virtual ~Column() {}

	inline const COLUMN_TYPE& type() { return type_; }
	inline const std::wstring& name() { return name_; }
	inline const bool& is_null() { return is_null_; }
	
protected:
	enum class COLUMN_TYPE
	{
		CHAR = 1,
		NUMERIC,
		DECIMAL,
		INT,
		SMALLINT,
		FLOAT,
		REAL,
		DOUBLE,
		DATETIME,
		VARCHAR = 12,
	};

private:
	COLUMN_TYPE type_;
	std::wstring name_;
	bool is_null_;
};

class IntegerColumn : public Column
{
public:
	IntegerColumn(COLUMN_TYPE_INTEGER type, std::wstring name, bool is_null)
		: Column(static_cast<COLUMN_TYPE>(type), name, is_null), value_(0)
	{}

private:
	INT64 value_;
};

class DecimalColumn : public Column
{
public:
	DecimalColumn(COLUMN_TYPE_DECIMAL type, std::wstring name, bool is_null)
		: Column(static_cast<COLUMN_TYPE>(type), name, is_null), value_(0.0)
	{}

private:
	double value_;
};

class StringColumn : public Column
{
public:
	StringColumn(COLUMN_TYPE_STRING type, std::wstring name, bool is_null)
		: Column(static_cast<COLUMN_TYPE>(type), name, is_null)
	{}

private:

	std::wstring value_;
};



class Table : public object
{


public:
	Table(std::wstring&& name)
		: name_(name)
	{}



private:

	std::wstring name_;
};

class QueryBase abstract
{
public:
	//bool Attach(COLUMN_TYPE type, std::wstring&& column_name, size_t size = 1);

private:
};
