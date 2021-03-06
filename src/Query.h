#pragma once

namespace DB_DATA
{
	enum class DATA_TYPE
	{
		CHAR = 1,
		NUMERIC,
		DECIMAL,
		INTEGER,
		SMALLINT,
		FLOAT,
		REAL,
		DOUBLE,
		DATETIME,
		VARCHAR = 12,
		DATE = 91,
		TIME,
		TIMESTAMP,
	};

	enum class USAGE_TYPE
	{
		DATA_IN,
		DATA_OUT,
		DATA_INOUT,
	};

	struct ColumnOption
	{
		ColumnOption(DATA_TYPE data_type, USAGE_TYPE usage_type)
			: _data_type(data_type), _usage_type(usage_type)
		{}

		ColumnOption(const ColumnOption& option)
			: _data_type(option._data_type), _usage_type(option._usage_type)
		{}

		const DATA_TYPE _data_type;
		const USAGE_TYPE _usage_type;
	};
	/*
	template<typename _Ty>
	using column_type_restrict =  typename std::enable_if_t
		<
			std::is_same<float, _Ty>::value ||
			std::is_same<double, _Ty>::value ||
			std::is_same<BYTE, _Ty>::value ||
			std::is_same<WORD, _Ty>::value ||
			std::is_same<std::string, _Ty>::value ||
			std::is_same<std::wstring, _Ty>::value
		>*;

	template<typename _Ty, column_type_restrict<_Ty>>
	struct ColumnData
	{
		Column(DB_DATA::ColumnOption option)
			: _option(option)
		{}

		DB_DATA::ColumnOption _option;

	};
	*/
}

class QueryBase abstract
{
public:
	QueryBase() {}
	virtual ~QueryBase() {}

protected:
	virtual bool Execute(bool timeout) abstract;

private:
		
};

class DynamicQuery : public QueryBase
{
public:
	struct Column;
	enum class QUERY_TYPE
	{
		QUERY_TYPE_SELECT,
		QUERY_TYPE_INSERT,
		QUERY_TYPE_DELETE,
		QUERY_TYPE_UPDATE
	};

	DynamicQuery(QUERY_TYPE type, std::wstring table_name);
	virtual ~DynamicQuery();

	bool RecordColumn(std::wstring column_name, DB_DATA::ColumnOption option);

	virtual bool Execute(bool timeout) override;

private:
	

	const QUERY_TYPE _type;
	const std::wstring _table_name;
};

//SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)L"SELECT seq, id, name, cal FROM SUSA_DT.dbo.TB_USER", SQL_NTS))

/*
Return Query

DECLARE @PARAM NVARCHAR(MAX)
		DECLARE @RADIO_ID INT = 0
		DECLARE @INPUT_TYPE INT = 0
		DECLARE @INPUT_COUNT INT = 0

		DECLARE @TEMP_QUERY NVARCHAR(MAX)

		SET @TEMP_QUERY = 'SELECT '
		SET @TEMP_QUERY += '@RADIO_ID = JSON_VALUE(@json_text, ''$.RADIO_INPUTS[' + CONVERT(NVARCHAR(MAX), @COUNT_IDX - 1) + '].RADIO_ID''),'
		SET @TEMP_QUERY += '@INPUT_TYPE = JSON_VALUE(@json_text, ''$.RADIO_INPUTS[' + CONVERT(NVARCHAR(MAX), @COUNT_IDX - 1) + '].INPUT_TYPE''),'
		SET @TEMP_QUERY += '@INPUT_COUNT = JSON_VALUE(@json_text, ''$.RADIO_INPUTS[' + CONVERT(NVARCHAR(MAX), @COUNT_IDX - 1) + '].INPUT_COUNT'')'

		SET @PARAM = '@json_text NVARCHAR(MAX), @COUNT_IDX NVARCHAR(MAX), @RADIO_ID NUMERIC(5,2) OUTPUT, @INPUT_TYPE NUMERIC(5,2) OUTPUT, @INPUT_COUNT NUMERIC(5,2) OUTPUT'
		EXEC SP_EXECUTESQL @TEMP_QUERY,
			@PARAM,
				@json_text, @COUNT_IDX,
				@RADIO_ID OUTPUT, @INPUT_TYPE OUTPUT, @INPUT_COUNT OUTPUT


*/

/*
TABLE ANALYZE

SELECT
	CONVERT(NVARCHAR(128), A.TABLE_CATALOG) AS DATABASE_NAME,
	CONVERT(NVARCHAR(128), A.TABLE_SCHEMA) AS SCHEMA_NAME,
	CONVERT(NVARCHAR(128), A.TABLE_NAME) AS TABLE_NAME,
	CONVERT(NVARCHAR(128), A.TABLE_TYPE) AS TABLE_TYPE,
	CONVERT(NVARCHAR(128), A.TABLE_CATALOG + '.' + A.TABLE_SCHEMA + '.' + A.TABLE_NAME) AS TABLE_PATH,
	CONVERT(NVARCHAR(128), MAX(ORDINAL_POSITION)) AS COLUMN_COUNT
	FROM INFORMATION_SCHEMA.TABLES AS A
		INNER JOIN INFORMATION_SCHEMA.COLUMNS AS B
			ON	A.TABLE_CATALOG = B.TABLE_CATALOG AND
				A.TABLE_SCHEMA = B.TABLE_SCHEMA AND
				A.TABLE_NAME = B.TABLE_NAME
		WHERE TABLE_TYPE = 'BASE TABLE'
			GROUP BY A.TABLE_CATALOG, A.TABLE_SCHEMA, A.TABLE_NAME, A.TABLE_TYPE
			ORDER BY DATABASE_NAME, SCHEMA_NAME, TABLE_TYPE, TABLE_NAME


SELECT
	CONVERT(NVARCHAR(128), A.TABLE_CATALOG) AS DATABASE_NAME,
	CONVERT(NVARCHAR(128), A.TABLE_SCHEMA) AS SCHEMA_NAME,
	CONVERT(NVARCHAR(128), A.TABLE_NAME) AS TABLE_NAME,
	CONVERT(NVARCHAR(128), A.TABLE_CATALOG + '.' + A.TABLE_SCHEMA + '.' + A.TABLE_NAME) AS TABLE_PATH,
	CONVERT(NVARCHAR(128), COLUMN_NAME) AS COLUMN_NAME,
	CONVERT(NVARCHAR(128), ORDINAL_POSITION) AS COLUMN_POSITION,
	CONVERT(NVARCHAR(128), IS_NULLABLE) AS IS_NULLABLE,
	CONVERT(NVARCHAR(128), DATA_TYPE) AS DATA_TYPE,
	ISNULL(CHARACTER_MAXIMUM_LENGTH, 0) AS CHAR_SIZE, -- -1 = MAX
	ISNULL(CHARACTER_OCTET_LENGTH, 0) AS WCHAR_SIZE, -- -1 = MAX
	CONVERT(NVARCHAR(128), ISNULL(CHARACTER_SET_NAME, '')) AS CHAR_TYPE
FROM INFORMATION_SCHEMA.COLUMNS AS A
	INNER JOIN INFORMATION_SCHEMA.TABLES AS B
		ON	A.TABLE_CATALOG = B.TABLE_CATALOG AND
			A.TABLE_SCHEMA = B.TABLE_SCHEMA AND
			A.TABLE_NAME = B.TABLE_NAME AND
			B.TABLE_TYPE = 'BASE TABLE'


SELECT S.NAME, I.TYPE_DESC, T.NAME, I.NAME, C.NAME FROM SYS.TABLES T
INNER JOIN SYS.SCHEMAS S ON T.SCHEMA_ID = S.SCHEMA_ID
INNER JOIN SYS.INDEXES I ON I.OBJECT_ID = T.OBJECT_ID
INNER JOIN SYS.INDEX_COLUMNS IC ON IC.OBJECT_ID = T.OBJECT_ID
INNER JOIN SYS.COLUMNS C ON C.OBJECT_ID = T.OBJECT_ID AND
		IC.COLUMN_ID = C.COLUMN_ID

WHERE I.IS_DISABLED = 0-- AND I.NAME IS NULL
ORDER BY T.NAME, IC.KEY_ORDINAL

*/