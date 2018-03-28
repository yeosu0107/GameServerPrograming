#define WIN32_LEAN_AND_MEAN  
#define INITGUID

#include <WinSock2.h>
#include <windows.h>  
#include <windowsx.h>
#include <iostream>
#include <thread>
#include <vector>

#include "protocol.h"

using namespace std;

HANDLE		g_iocp;

struct EXOver {
	WSAOVERLAPPED wsaover;
	bool is_recv;
	WSABUF wsabuf;
	char io_buf[MAX_BUFF_SIZE];
};

struct CLIENT {
	SOCKET s;
	bool is_use;
	char x, y;
	EXOver exover;
};

CLIENT g_clients[MAX_USER];

void Initialize() {
	for (auto& p : g_clients) {
		p.is_use = false;
		p.exover.is_recv = true;
		p.exover.wsabuf.buf = p.exover.io_buf;
		p.exover.wsabuf.len = sizeof(p.exover.io_buf);
	}

	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
}

void WorkerThread() 
{
	while (true) {
		unsigned long			data_size;
		unsigned long long	key;
		WSAOVERLAPPED*	p_over;

		BOOL is_success = GetQueuedCompletionStatus(g_iocp, 
			&data_size, &key, &p_over, INFINITE);
		// 俊矾 贸府
		if (is_success == FALSE) {
			cout << "Error is GQCS key [" << key << "]" << endl;
			continue;
		}
		// 立加 辆丰 贸府
		if (data_size == 0) {
			closesocket(g_clients[key].s);
			g_clients[key].is_use = false;
			continue;
		}
		// Send / Recv 贸府

	}
}

void AcceptThread() 
{
	auto g_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	
	SOCKADDR_IN bind_addr;
	ZeroMemory(&bind_addr, sizeof(SOCKADDR_IN));
	bind_addr.sin_family = AF_INET;
	bind_addr.sin_port = htons(MY_SERVER_PORT);
	bind_addr.sin_addr.s_addr = INADDR_ANY;

	::bind(g_socket, reinterpret_cast<sockaddr*>(&bind_addr), 
		sizeof(SOCKADDR_IN));

	listen(g_socket, 1000);

	while (true) {
		SOCKADDR_IN c_addr;
		ZeroMemory(&c_addr, sizeof(SOCKADDR_IN));
		c_addr.sin_family = AF_INET;
		c_addr.sin_port = htons(MY_SERVER_PORT);
		c_addr.sin_addr.s_addr = INADDR_ANY;
		int c_addr_len = sizeof(SOCKADDR_IN);

		auto new_socket = WSAAccept(g_socket, reinterpret_cast<SOCKADDR*>(&c_addr),
			&c_addr_len, NULL, NULL);

		int new_key = -1;

		for (int i = 0; i < MAX_USER; ++i) {
			if (!g_clients[i].is_use) {
				new_key = i;
				break;
			}
		}
		if (new_key == -1) {
			cout << "MAX USER EXCEEDED!!\n" << endl;
			continue;
		}
		g_clients[new_key].s = new_socket;
		g_clients[new_key].x = 4;
		g_clients[new_key].y = 4;
		ZeroMemory(&g_clients[new_key].exover.wsaover, sizeof(WSAOVERLAPPED));

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(new_socket), 
			g_iocp, new_key, 0); 

		g_clients[new_key].is_use = true;

		unsigned long flag = 0;
		WSARecv(new_socket, &g_clients[new_key].exover.wsabuf, 1, NULL, &flag, 
			&g_clients[new_key].exover.wsaover, NULL);
	}
}

int main(void)
{	
	vector<thread> all_threads;
	Initialize();

	for (int i = 0; i < 4; ++i) {
		all_threads.push_back(thread(WorkerThread));
	}
	all_threads.push_back(thread(AcceptThread));

	for (auto& th : all_threads) {
		th.join();
	}
	WSACleanup();
}