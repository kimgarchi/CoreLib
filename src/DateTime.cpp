#include "stdafx.h"
#include "DateTime.h"

DateTime::DateTime()
	: _time(NULL)
{
}

DateTime::DateTime(const time_t& date_time)
	: _time(date_time)
{
}

DateTime::DateTime(const DateTime& date_time)
	: _time(date_time._time)
{
}

DateTime::DateTime(const std::string date_time)
	: _time(NULL)
{
}

DateTime::DateTime(const std::wstring date_time)
	: _time(NULL)
{
}
