#pragma once
#include "stdafx.h"
#include <locale.h>

#define UNICODE  
#include <sqlext.h>  

#define NAME_LEN 50 

void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

const int SEARCH_ID		= 0;
const int UPDATE_POS	= 1;

class DBConnect {
private:
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	

	std::wstring searchUser = L"dbo.search_user_id ";
	std::wstring updatePos = L"dbo.update_user_pos ";
public:
	DBConnect();
	~DBConnect();

	bool SearchUserAndLogin(int id, int& x, int& y);
	void UpdateUserPos(int id, int x, int y);
};
