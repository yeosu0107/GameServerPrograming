#pragma once
#define WIN32_LEAN_AND_MEAN  
#define INITGUID

#include <windows.h>  
#include <windowsx.h>
#include <iostream>
#include <WinSock2.h>
#include <chrono>

#include "protocol.h"
#include "Client.h"

#pragma comment (lib, "ws2_32.lib")

void err_display(const char* msg, int err_no);