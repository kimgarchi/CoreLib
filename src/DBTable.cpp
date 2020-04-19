

#include "DBTable.h"

DBTable::DBTable(std::wstring name, TABLE_TYPE type)
	: name_(name), type_(type)
{
}

DBTable::~DBTable()
{
	columns_.clear();
	index_by_type_.clear();
}

bool DBTable::AttachColumn(Column&& column)
{
	if (column_by_name_.find(column.name) != column_by_name_.end())
		return false;

	columns_.emplace_back(column);
	return column_by_name_.emplace(column.name, column).second;
}

bool DBTable::AttachIndex(std::wstring name, TABLE_INDEX_TYPE type, Columns&& columns)
{
	auto type_itor = index_by_type_.find(type);
	if (type_itor != index_by_type_.end())
		return false;

	auto& indexes = type_itor->second;

	const auto& name_itor = indexes.find(name);
	if (name_itor != indexes.end())
		return false;

	return indexes.emplace(name, columns).second;;
}
