#pragma once
#include "stdafx.h"
#include <sqltypes.h>
#include <sql.h>
#include <sqlext.h>

#define SQL_RETURN_CODE_LEN 1024

class ODBC
{
public:
	ODBC();
	virtual ~ODBC();	

	bool Cleanup();
	SQLRETURN Connect(std::wstring address, std::wstring port, std::wstring database, std::wstring id, std::wstring password);
	
private:
	SQLHANDLE connect_;
	SQLHANDLE statement_;
	SQLHANDLE environment_;
};