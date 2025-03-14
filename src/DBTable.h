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
	Column(std::wstring name, COLUMN_TYPE type, bool is_null)
		: name_(name), type_(type), is_null_(is_null)
	{}

	Column(const Column& column)
		: name_(column.name_), type_(column.type_), is_null_(column.is_null_)
	{}

	virtual ~Column() {}

	inline const COLUMN_TYPE& type() { return type_; }
	inline const std::wstring& name() { return name_; }
	inline const bool& is_null() { return is_null_; }

	inline bool is_valid() { return name_.compare(L"") != 0; }

	bool AttachIndex(std::wstring&& index_name) { return bind_index_names_.emplace(index_name).second; }

private:
	const std::wstring name_;
	const COLUMN_TYPE type_;
	const bool is_null_;
	BindIndexNames bind_index_names_;
};

enum class TABLE_TYPE
{
	BASIC,
	VIEW,
};

enum class TABLE_INDEX_TYPE
{
	CLUSTERED,
	NONCLUSTERED,
	HEAP,
};

class DBRecord
{
public:

private:

};

class DBTable
{
private:
	using Columns = std::vector<Column>;
	using ColumnByName = std::map<std::wstring, Column>;
	using Index = std::map<std::wstring, Columns>;
	using IndexByType = std::map<TABLE_INDEX_TYPE, Index>;

public:
	DBTable(std::wstring name, TABLE_TYPE type = TABLE_TYPE::BASIC);
	~DBTable();

	bool AttachColumn(const std::wstring& name __in, Column& column __in);
	bool AttachIndex(std::wstring name, TABLE_INDEX_TYPE type, Columns& columns);

private:
	const std::wstring name_;
	const TABLE_TYPE type_;

	Columns columns_;
	ColumnByName column_by_name_;
	IndexByType index_by_type_;
};

