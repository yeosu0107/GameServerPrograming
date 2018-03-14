#pragma once

#include "targetver.h"

#pragma comment(lib, "ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <Windows.h>
#include <iostream>

const int WindowWidth = 500;
const int WindowHeight = 500;
#define MAX_BUF 30

const DWORD DIR_LEFT		= 0x01;
const DWORD DIR_RIGHT	= 0x02;
const DWORD DIR_UP			= 0x04;
const DWORD DIR_DOWN	= 0x08;

using namespace std;

#pragma pack(1)
struct RecvDataFormat {
	WORD	m_xPos;		//xÁÂÇ¥
	WORD	m_yPos;		//yÁÂÇ¥

	RecvDataFormat() : m_xPos(0), m_yPos(0)
	{ }
};
#pragma pack()

int recvn(SOCKET s, char *buf, int len, int flags);
void error_quit(char* msg);
void error_display(char *msg);