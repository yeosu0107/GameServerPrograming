#pragma once

#include <windows.h>   // include important windows stuff
#include <windowsx.h> 
#include <iostream>

#include <d3d9.h>  // directX includes
#include <d3dx9tex.h>  // directX includes

#include <queue>

#include "gpdumb1.h"

using namespace std;

struct textMSG {
	DWORD time;
	WCHAR msg[300];
	textMSG(DWORD _time) {
		time = _time;
	}
	textMSG(const textMSG& T) {
		this->time = T.time;
		wcsncpy_s(this->msg, T.msg, 256);
	}
};

struct textManager
{
	queue<textMSG> m_message;

	void insert(DWORD time, WCHAR* msg);

	void draw();
};