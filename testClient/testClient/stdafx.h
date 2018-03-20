#pragma once

#include "targetver.h"

#pragma comment(lib, "ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <Windows.h>
#include <iostream>

const int WindowWidth = 500;
const int WindowHeight = 500;
#define MAX_BUF 512

const DWORD DIR_LEFT		= 0x01;
const DWORD DIR_RIGHT	= 0x02;
const DWORD DIR_UP			= 0x04;
const DWORD DIR_DOWN	= 0x08;

using namespace std;

int recvn(SOCKET s, char *buf, int len, int flags);
void error_quit(char* msg);
void error_display(char *msg);

static int				clientNum = 1;

#pragma pack(1)
struct PlayerInfo {
	int	m_playerNum;
	int xPos;
	int yPos;

	PlayerInfo() {
		m_playerNum = 0;
		xPos = 0;
		yPos = 0;
	}

	PlayerInfo(int num, int x, int y) {
		m_playerNum = num;
		xPos = x;
		yPos = y;
	}

	void set(int num, int x, int y) {
		m_playerNum = num;
		xPos = x;
		yPos = y;
	}
	void print() {
		printf("%d	%d	%d\n", m_playerNum, xPos, yPos);
	}
};
#pragma pack()
static PlayerInfo playerInfo[10];