#define WIN32_LEAN_AND_MEAN  
#define INITGUID

#include <WinSock2.h>
#include <windows.h>  
#include <windowsx.h>
#include <iostream>
#include <thread>
#include <vector>
#include <unordered_set>
#include <mutex>

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

class Object {
public:
	SOCKET s;
	bool is_use = false;
	int x, y;

	int zone_x, zone_y;
};

class Client : public Object{
public:
	unordered_set<int> viewlist;
	mutex vlm;

	//for io func
	EXOver exover;
	int packet_size;
	int prev_size;
	char prev_packet[MAX_PACKET_SIZE];

	Client() {
		exover.is_recv = true;
		exover.wsabuf.buf = exover.io_buf;
		exover.wsabuf.len = sizeof(exover.io_buf);
		packet_size = 0;
		prev_size = 0;
	}
};

//Client g_clients[NUM_OF_NPC];
//array<Client, NUM_OF_NPC> g_clients;
vector<Client> g_clients(NUM_OF_NPC);

unordered_set<int> g_zone[20][20];

//CLIENT g_clients[NUM_OF_NPC];

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

bool CanSee(int cl1, int cl2) {
	int dist_sq = (g_clients[cl1].x - g_clients[cl2].x)*(g_clients[cl1].x - g_clients[cl2].x) +
		(g_clients[cl1].y - g_clients[cl2].y)*(g_clients[cl1].y - g_clients[cl2].y);
	return (dist_sq <= VIEW_RADIUS * VIEW_RADIUS);
}

void Initialize() {
	wcout.imbue(locale("korean"));

	for (int i = NPC_START; i < NUM_OF_NPC; ++i) {
		g_clients[i].is_use = true;
		g_clients[i].x = rand() % BOARD_WIDTH;
		g_clients[i].y = rand() % BOARD_HEIGHT;

		g_clients[i].zone_x = g_clients[i].x / 20;
		g_clients[i].zone_y = g_clients[i].y / 20;

		g_zone[g_clients[i].zone_y][g_clients[i].zone_x].insert(i);
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

void SendPutObject(int client, int object) {
	sc_packet_put_player p;
	p.id = object;
	p.size = sizeof(p);
	p.type = SC_PUT_PLAYER;
	p.x = g_clients[object].x;
	p.y = g_clients[object].y;
	
	SendPacket(client, &p);
}

void SendRemoveObject(int client, int object) {
	sc_packet_remove_player p;
	p.id = object;
	p.size = sizeof(p);
	p.type = SC_REMOVE_PLAYER;

	SendPacket(client, &p);
}

void DisconnectPlayer(int id) {
	closesocket(g_clients[id].s);
	
	cout << "Client [" << id << "] DisConnected\n";

	sc_packet_remove_player p;
	p.id = id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_PLAYER;

	g_clients[id].vlm.lock();
	unordered_set<int> vl_copy = g_clients[id].viewlist;
	g_clients[id].viewlist.clear();
	g_clients[id].vlm.unlock();

	for (auto& i : vl_copy) {
		g_clients[i].vlm.lock();
		if (g_clients[i].is_use) {
			if (g_clients[i].viewlist.count(id) != 0) {
				g_clients[i].viewlist.erase(id);
				g_clients[i].vlm.unlock();
				SendPacket(i, &p);
			}
		}
		else {
			g_clients[i].vlm.unlock();
		}
	}
	g_clients[id].is_use = false;
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
	int prev_x = g_clients[clientID].zone_x;
	int prev_y = g_clients[clientID].zone_y;

	g_clients[clientID].zone_x = g_clients[clientID].x / 20;
	g_clients[clientID].zone_y = g_clients[clientID].y / 20;

	if (g_clients[clientID].zone_x != prev_x || g_clients[clientID].zone_y != prev_y) {
		g_zone[prev_y][prev_x].erase(clientID);
		g_zone[g_clients[clientID].zone_y][g_clients[clientID].zone_x].insert(clientID);
	}

	sc_packet_pos posPacket;
	posPacket.id = clientID;
	posPacket.size = sizeof(sc_packet_pos);
	posPacket.type = SC_POS;
	posPacket.x = g_clients[clientID].x;
	posPacket.y = g_clients[clientID].y;

	unordered_set<int> new_viewList;

	for (auto& i : g_zone[g_clients[clientID].zone_y][g_clients[clientID].zone_x]) {
		if (i == clientID) continue;
		if (!g_clients[i].is_use) continue;

		if (!CanSee(clientID, i)) continue;
		new_viewList.insert(i);
	}

	//for (int i = 0; i < NUM_OF_NPC; ++i) {
	//	if (i == clientID) continue;
	//	if (!g_clients[i].is_use) continue;

	//	if (!CanSee(clientID, i)) continue;
	//	new_viewList.insert(i);
	//}
	
	for (auto& id : new_viewList) {
		//새로 viewlist에 들어오는 객체 처리
		g_clients[clientID].vlm.lock();
		if (g_clients[clientID].viewlist.count(id) == 0) {
			g_clients[clientID].viewlist.insert(id);
			g_clients[clientID].vlm.unlock();
			SendPutObject(clientID, id);

			if (id >= NPC_START)
				continue;
			g_clients[id].vlm.lock();
			if (g_clients[id].viewlist.count(clientID) == 0) {
				g_clients[id].viewlist.insert(clientID);
				g_clients[id].vlm.unlock();
				SendPutObject(id, clientID);
			}
			else {
				g_clients[id].vlm.unlock();
				SendPacket(id, &posPacket);
			}
		}
		else {
			g_clients[clientID].vlm.unlock();
			if (id >= NPC_START)
				continue;
			//view에 계속 남아있는 객체 처리
			g_clients[id].vlm.lock();
			if (g_clients[id].viewlist.count(clientID) == 0) {
				g_clients[id].viewlist.insert(clientID);
				g_clients[id].vlm.unlock();
				SendPutObject(id, clientID);
			}
			else {
				g_clients[id].vlm.unlock();
				SendPacket(id, &posPacket);
			}
		}
	}
	//viewlist에서 나가는 객체 처리
	vector<int> tmpDeleteList;
	g_clients[clientID].vlm.lock();
	unordered_set<int> old_vl = g_clients[clientID].viewlist;
	g_clients[clientID].vlm.unlock();
	for (auto& id : old_vl) {
		if (0 == new_viewList.count(id)) {
			tmpDeleteList.emplace_back(id);
			if (id >= NPC_START)
				continue;
			g_clients[id].vlm.lock();
			if (0 != g_clients[id].viewlist.count(clientID)) {
				g_clients[id].viewlist.erase(clientID);
				g_clients[id].vlm.unlock();
				SendRemoveObject(id, clientID);
			}
			else
				g_clients[id].vlm.unlock();
			
		}
	}


	for (auto& delid : tmpDeleteList) {
		g_clients[clientID].vlm.lock();
		g_clients[clientID].viewlist.erase(delid);
		g_clients[clientID].vlm.unlock();
		SendRemoveObject(clientID, delid);
	}

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
			DisconnectPlayer(key);
			continue;
		}
		// 접속 종료 처리
		if (data_size == 0) {
			DisconnectPlayer(key);
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
		g_clients[new_key].x = 10;
		g_clients[new_key].y = 10;
		ZeroMemory(&g_clients[new_key].exover.wsaover, sizeof(WSAOVERLAPPED));

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(new_socket), 
			g_iocp, new_key, 0); 

		g_clients[new_key].viewlist.clear();
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
		p.x = g_clients[new_key].x;
		p.y = g_clients[new_key].y;

		g_clients[new_key].zone_x = p.x / 20;
		g_clients[new_key].zone_y = p.y / 20;

		g_zone[g_clients[new_key].zone_y][g_clients[new_key].zone_x].insert(new_key);

		//나의 접속을 다른 플레이어에게 알림 (나를 포함)
		//같은 존에 있는 플레이어에만 알림
		for (auto& i : g_zone[g_clients[new_key].zone_y][g_clients[new_key].zone_x]) {
			if (i >= MAX_USER) //user가 아니면 무시
				continue;
			if (g_clients[i].is_use) {
				if (!CanSee(i, new_key))
					continue;
				g_clients[i].vlm.lock();
				if (i != new_key)
					g_clients[i].viewlist.insert(new_key);
				g_clients[i].vlm.unlock();
				SendPacket(i, &p);
			}
		}

		//나에게 접속중인 다른 플레이어의 정보를 전송
		//(NPC, 플레이어 포함)
		for (auto& i : g_zone[g_clients[new_key].zone_y][g_clients[new_key].zone_x]) {
			if (g_clients[i].is_use) {
				if (i == new_key)
					continue;
				if (!CanSee(i, new_key))
					continue;
				p.id = i;
				p.x = g_clients[i].x;
				p.y = g_clients[i].y;
				g_clients[new_key].vlm.lock();
				g_clients[new_key].viewlist.insert(i);
				g_clients[new_key].vlm.unlock();
				SendPacket(new_key, &p);
			}
		}

		/*for (int i = 0; i < MAX_USER; ++i) {
			if (g_clients[i].is_use) {
				if (i == new_key)
					continue;
				if (!CanSee(i, new_key))
					continue;
				p.id = i;
				p.x = g_clients[i].x;
				p.y = g_clients[i].y;
				g_clients[new_key].vlm.lock();
				g_clients[new_key].viewlist.insert(i);
				g_clients[new_key].vlm.unlock();
				SendPacket(new_key, &p);
			}
		}*/
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