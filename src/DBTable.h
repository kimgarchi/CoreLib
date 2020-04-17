#pragma once
#include "stdafx.h"
#include <sql.h>

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

using BindIndexNames = std::set<std::wstring>;
class Column
{
public:
	Column(COLUMN_TYPE type, std::wstring name, BYTE position, bool is_null)
		: type_(type), name_(name), position_(position), is_null_(is_null)
	{}

	virtual ~Column() {}

	inline const COLUMN_TYPE& type() { return type_; }
	inline const std::wstring& name() { return name_; }
	inline const bool& is_null() { return is_null_; }

	bool AttachIndex(std::wstring&& index_name) { return bind_index_names_.emplace(index_name).second; }

private:
	const COLUMN_TYPE type_;
	const std::wstring name_;
	const bool is_null_;
	const BYTE position_;
	BindIndexNames bind_index_names_;
};

enum class TABLE_INDEX
{
	CLUSTERED,
	NONCLUSTERED,
	HEAP,
};

class DBTable
{
public:

private:
	using Columns = std::vector<Column>;
	using Index = std::map<TABLE_INDEX, Columns>;	
	
	Columns columns_;
};

