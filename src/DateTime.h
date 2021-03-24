#pragma once
#include <ctime>

class DateTime 
{
public:
	DateTime();
	DateTime(const time_t& date_time);
	DateTime(const DateTime& date_time);
	DateTime(const std::string date_time);
	DateTime(const std::wstring date_time);

private:
	const time_t _time;
};

