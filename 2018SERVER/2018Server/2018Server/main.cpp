#define WIN32_LEAN_AND_MEAN  
#define INITGUID

#include <WinSock2.h>
#include <windows.h>  
#include <windowsx.h>
#include <iostream>
#include <thread>
#include <vector>

#include "protocol.h"

#pragma comment (lib, "ws2_32.lib")

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
	int packet_size;
	int prev_size;
	char prev_packet[MAX_PACKET_SIZE];
};

CLIENT g_clients[MAX_USER];

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
	wcout << L"에러 " << lpMsgBuf << endl;
	LocalFree(lpMsgBuf);
}

void Initialize() {
	wcout.imbue(locale("korean"));
	//wcout << L"한글 메세지 출력 테스트\n";

	for (auto& p : g_clients) {
		p.is_use = false;
		p.exover.is_recv = true;
		p.exover.wsabuf.buf = p.exover.io_buf;
		p.exover.wsabuf.len = sizeof(p.exover.io_buf);
		p.packet_size = 0;
		p.prev_size = 0;
	}

	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
}

void SendPacket(int id, void* packet) {
	EXOver* over = new EXOver;
	char* p = reinterpret_cast<char*>(packet);
	memcpy(over->io_buf, p, p[0]);
	over->is_recv = false;
	over->wsabuf.buf = over->io_buf;
	over->wsabuf.len = p[0];

	ZeroMemory(&over->wsaover, sizeof(WSAOVERLAPPED));

	int ret = WSASend(g_clients[id].s, &over->wsabuf, 1, NULL,
		0, &over->wsaover, NULL);

	if (ret != 0) {
		int err_no = WSAGetLastError();
		if(WSA_IO_PENDING!=err_no) //에러코드가 이것이면 에러가 아니다. (send가 끝났는데 계속 send인 경우)
			err_display("Error in SendPacket : ", err_no);
	}

	cout << "SendPacket to Client [" << id << "] type [" 
		<< (int)p[1] << "] size [" << (int)p[0] << "]\n";
}

void ProcessPacket(int clientID, char* packet) {
	cs_packet_up* p = reinterpret_cast<cs_packet_up*>(packet);

	switch (p->type) {
	case CS_UP:
		g_clients[clientID].y -= 1;
		if (g_clients[clientID].y < 0)
			g_clients[clientID].y = 0;
		break;
	case CS_DOWN:
		g_clients[clientID].y += 1;
		if (g_clients[clientID].y >= BOARD_HEIGHT)
			g_clients[clientID].y = BOARD_HEIGHT - 1;
		break;
	case CS_LEFT:
		g_clients[clientID].x -= 1;
		if (g_clients[clientID].x < 0)
			g_clients[clientID].x = 0;
		break;
	case CS_RIGHT:
		g_clients[clientID].x += 1;
		if (g_clients[clientID].x >= BOARD_WIDTH)
			g_clients[clientID].x = BOARD_WIDTH - 1;
		break;
	default:
		cout << "Unknown Protocol from Client [" << clientID << "]\n";
		return;
	}

	sc_packet_pos posPacket;
	posPacket.id = clientID;
	posPacket.size = sizeof(sc_packet_pos);
	posPacket.type = SC_POS;
	posPacket.x = g_clients[clientID].x;
	posPacket.y = g_clients[clientID].y;

	SendPacket(clientID, &posPacket);
}

void WorkerThread() 
{
	while (true) {
		unsigned long			data_size;
		unsigned long long	key;
		WSAOVERLAPPED*	p_over;

		BOOL is_success = GetQueuedCompletionStatus(g_iocp, 
			&data_size, &key, &p_over, INFINITE);

		cout << "GQCS from client [" << key << "] with size [" << data_size << "]\n";

		// 에러 처리
		if (is_success == FALSE) {
			cout << "Error is GQCS key [" << key << "]\n";
			continue;
		}
		// 접속 종료 처리
		if (data_size == 0) {
			closesocket(g_clients[key].s);
			g_clients[key].is_use = false;
			continue;
		}
		// Send / Recv 처리
		EXOver* exover = reinterpret_cast<EXOver*>(p_over);
		if (exover->is_recv == true) {
			int recv_size = data_size;
			char* ptr = exover->io_buf;
			while (recv_size > 0) {
				if (g_clients[key].packet_size == 0) 
					g_clients[key].packet_size = ptr[0]; //패킷의 사이즈는 맨 앞
					//맨 처음 recv를 하거나, 모두 처리해서 새로운 패킷을 받아야 하는 경우
				
				int remain = g_clients[key].packet_size - g_clients[key].prev_size;
				if (remain <= recv_size) {
					//build packet
					memcpy(g_clients[key].prev_packet + g_clients[key].prev_size, 
						ptr, remain);
					//패킷 처리
					ProcessPacket(static_cast<int>(key), g_clients[key].prev_packet);
					recv_size -= remain;
					ptr += remain;
					g_clients[key].packet_size = 0;
					g_clients[key].prev_size = 0;
				}
				else {
					//아직 모든 패킷을 받지 않은 경우 재조립 수행
					memcpy(g_clients[key].prev_packet + g_clients[key].prev_size,
						ptr, recv_size);
					g_clients[key].prev_size += recv_size;
					recv_size -= recv_size;
					ptr += recv_size;
				}
			}

			unsigned long rflag = 0;
			ZeroMemory(&exover->wsaover, sizeof(WSAOVERLAPPED));
			int ret = WSARecv(g_clients[key].s, &exover->wsabuf, 1, NULL,
				&rflag, &exover->wsaover, NULL);

			if (ret != 0) {
				int err_no = WSAGetLastError();
				if (err_no != WSA_IO_PENDING)
					err_display("Recv in WorkThread", err_no);
			}
		}
		else {
			delete exover;
		}
	}
}

void AcceptThread() 
{
	auto g_socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	
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
		cout << "new Client Accepted!\n";
		int new_key = -1;

		for (int i = 0; i < MAX_USER; ++i) {
			if (!g_clients[i].is_use) {
				new_key = i;
				break;
			}
		}
		if (new_key == -1) {
			cout << "MAX USER EXCEEDED!!\n";
			continue;
		}
		cout << "New Client's ID : " << new_key << endl;
		g_clients[new_key].s = new_socket;
		g_clients[new_key].x = 4;
		g_clients[new_key].y = 4;
		ZeroMemory(&g_clients[new_key].exover.wsaover, sizeof(WSAOVERLAPPED));

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(new_socket), 
			g_iocp, new_key, 0); 

		g_clients[new_key].is_use = true;

		unsigned long flag = 0;
		int ret = WSARecv(new_socket, &g_clients[new_key].exover.wsabuf, 1, NULL, &flag, 
			&g_clients[new_key].exover.wsaover, NULL);

		if (ret != 0) {
			int err_no = WSAGetLastError();
			if (err_no != WSA_IO_PENDING)
				err_display("Recv in AcceptThread", err_no);
		}

		sc_packet_put_player p;
		p.id = new_key;
		p.size = sizeof(sc_packet_put_player);
		p.type = SC_PUT_PLAYER;
		p.x = 4;
		p.y = 4;
		SendPacket(new_key, &p);
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