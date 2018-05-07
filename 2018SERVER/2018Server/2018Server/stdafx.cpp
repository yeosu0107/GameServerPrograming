#include "stdafx.h"

void err_display(const char* msg, int err_no) {
	WCHAR * lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)& lpMsgBuf, 0, NULL);
	cout << msg;
	//printf("% s", msg);
	wcout << L"���� " << lpMsgBuf << endl;
	LocalFree(lpMsgBuf);
}