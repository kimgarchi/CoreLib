#include "stdafx.h"
#include "Query.h"

DynamicQuery::DynamicQuery(QUERY_TYPE type, std::wstring table_name)
    : _type(type), _table_name(table_name)
{
}

DynamicQuery::~DynamicQuery()
{
}

bool DynamicQuery::RecordColumn(std::wstring column_name, DB_DATA::ColumnOption option)
{


    return true;
}

bool DynamicQuery::Execute(bool timeout)
{
    return false;
}
