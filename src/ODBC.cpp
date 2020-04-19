#include "stdafx.h"
#include "ODBC.h"

ODBC::ODBC()
	: connect_(nullptr), statement_(nullptr), environment_(nullptr)
{
}

ODBC::~ODBC()
{
	Cleanup();
}

bool ODBC::Cleanup()
{
	if (statement_ != nullptr)
	{
		auto ret = SQLFreeHandle(SQL_HANDLE_STMT, statement_);
		if (ret != SQL_SUCCESS)
		{
			assert(false);
			return false;
		}
	}

	if (connect_ != nullptr)
	{
		auto ret = SQLDisconnect(connect_);
		if (ret != SQL_SUCCESS)
		{
			assert(false);
			return false;
		}

		ret = SQLFreeHandle(SQL_HANDLE_DBC, connect_);
		if (ret != SQL_SUCCESS)
		{
			assert(false);
			return false;
		}
	}

	if (environment_ != nullptr)
	{
		auto ret = SQLFreeHandle(SQL_HANDLE_ENV, environment_);
		if (ret != SQL_SUCCESS)
		{
			assert(false);
			return false;
		}
	}

	return true;
}

SQLRETURN ODBC::Connect(std::wstring address, std::wstring port, std::wstring database, std::wstring id, std::wstring password)
{
	if (Cleanup() == false)
	{
		assert(false);
		return SQL_ERROR;
	}

	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &environment_) != SQL_SUCCESS)
	{
		assert(false);
		return SQL_ERROR;
	}

	if (SQLSetEnvAttr(environment_, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0) != SQL_SUCCESS)
	{
		assert(false);
		return SQL_ERROR;
	}

	if (SQLAllocHandle(SQL_HANDLE_DBC, environment_, &connect_) != SQL_SUCCESS)
	{
		assert(false);
		return SQL_ERROR;
	}

	std::wstringstream conn;
	conn << L"DRIVER={SQL Server};SERVER=" << address << L"," << port << L";DATABASE=" << database << L";UID=" << id << L";PWD=" << password << L";";

	SQLWCHAR ret_conn[SQL_RETURN_CODE_LEN];
	return SQLDriverConnect(connect_, NULL, (SQLWCHAR*)conn.str().c_str(), SQL_NTS, ret_conn, SQL_RETURN_CODE_LEN, NULL, SQL_DRIVER_NOPROMPT);

	/*
	if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)L"SELECT seq, id, name, cal FROM SUSA_DT.dbo.TB_USER", SQL_NTS))
	{
		std::cout << "Error querying SQL Server";
		std::cout << "\n";
		goto COMPLETED;
	}
	else
	{
		while (SQLFetch(sqlStmtHandle) == SQL_SUCCESS)
		{
			SQLINTEGER seq = 0;
			SQLINTEGER id = 0;
			SQLWCHAR name[32];
			SQLFLOAT cal = 0.0f;

			SQLGetData(sqlStmtHandle, 1, SQL_INTEGER, &seq, 0, NULL);
			SQLGetData(sqlStmtHandle, 2, SQL_INTEGER, &id, 0, NULL);
			SQLGetData(sqlStmtHandle, 3, SQL_WCHAR, name, 32, NULL);
			SQLGetData(sqlStmtHandle, 4, SQL_FLOAT, &cal, 0, NULL);

			std::cout << "\nQuery Result:\n\n";
			std::cout << seq << std::endl;
			std::cout << id << std::endl;
			std::wcout << std::wstring(name).c_str() << std::endl;
			std::cout << cal << std::endl;
		}
	}
	*/
}

SQLHANDLE ODBC::Execute(DynamicQuery query)
{
	return SQLHANDLE();
}
