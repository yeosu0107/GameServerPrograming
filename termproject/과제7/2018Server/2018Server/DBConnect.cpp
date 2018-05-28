#include "DBConnect.h"
#include <string>

void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER  iError;
	WCHAR       wszMessage[1000];
	WCHAR       wszState[SQL_SQLSTATE_SIZE + 1];

	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT *)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}

DBConnect::DBConnect()
{
	// Allocate environment handle  
	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	// Set the ODBC version environment attribute 
	SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
	// Allocate connection handle  
	SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
	// Set login timeout to 5 seconds  
	SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
	// Connect to data source
	SQLConnect(hdbc, (SQLWCHAR*)L"ODBC_2012180023", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);
	// Allocate statement handle 
	SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
}

DBConnect::~DBConnect()
{
	SQLCancel(hstmt);
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

bool DBConnect::SearchUserAndLogin(int id, int & x, int & y)
{
	SQLRETURN retcode;
	SQLWCHAR szUser_Name[NAME_LEN];
	SQLINTEGER nID, nUser_Level, nX, nY;
	SQLLEN cbName = 0, cbID = 0, cbLevel = 0, cbX = 0, cbY = 0;

	wstring user_id = to_wstring(id);
	wstring val = searchUser + user_id;

	retcode = SQLExecDirect(hstmt, (SQLWCHAR *)(val.c_str()), SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		SQLBindCol(hstmt, 1, SQL_C_LONG, &nID, 100, &cbID);
		SQLBindCol(hstmt, 2, SQL_C_CHAR, szUser_Name, NAME_LEN, &cbName);
		SQLBindCol(hstmt, 3, SQL_C_LONG, &nUser_Level, 100, &cbLevel);
		SQLBindCol(hstmt, 4, SQL_C_LONG, &nX, 100, &cbX);
		SQLBindCol(hstmt, 5, SQL_C_LONG, &nY, 100, &cbY);
		retcode = SQLFetch(hstmt);
		SQLCloseCursor(hstmt);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			x = nX;
			y = nY;
			
			return true;
		}
		else 	return false;
		
	}
	else 
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
	
	SQLCloseCursor(hstmt);
	return false;
}

void DBConnect::UpdateUserPos(int id, int x, int y)
{
	SQLRETURN retcode;

	wchar_t tmp[100];
	wsprintf(tmp, L"%d, %d, %d", id, x, y);
	wstring tt(tmp);
	wstring val = updatePos + tt;

	retcode = SQLExecDirect(hstmt, (SQLWCHAR *)(val.c_str()), SQL_NTS);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
	}
}


