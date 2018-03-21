#pragma once
#pragma comment(lib, "ws2_32")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define SERVERPORT 9000

#include "targetver.h"

#include <WinSock2.h>
#include <iostream>
#include <Windows.h>

#include "ClientConnect.h"

using namespace std;

extern void err_display(const char *msg);
extern int recvn(SOCKET s, char *buf, int len, int flags);
extern void printData(char *data);



const DWORD DIR_LEFT = 0x01;
const DWORD DIR_RIGHT = 0x02;
const DWORD DIR_UP = 0x04;
const DWORD DIR_DOWN = 0x08;

extern ClientConnect* g_ClientArray[MAX_CLIENT];
extern UINT g_nClient;
extern PlayerInfo	g_Player[MAX_CLIENT];