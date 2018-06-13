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
	WCHAR* msg;
	textMSG(DWORD _time, WCHAR* _msg) {
		time = _time;
		msg = _msg;
	}
	textMSG(const textMSG& T) {
		this->time = T.time;
		this->msg = T.msg;
	}
};

struct textManager
{
	queue<textMSG> m_message;

	void insert(DWORD time, WCHAR* msg);

	void draw();
};