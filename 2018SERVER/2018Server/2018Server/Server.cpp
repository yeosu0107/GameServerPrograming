#include "stdafx.h"
#include "Server.h"


Server* Server::g_server = nullptr;

Server::~Server()
{
	WSACleanup();
	delete g_server;
}

Server * Server::getInstance()
{
	if (!g_server) {
		g_server = new Server();
	}
	return g_server;
}

void Server::Initialize()
{
	g_clients.resize(NUM_OF_NPC, Client());
	wcout.imbue(locale("korean"));

	for (int i = NPC_START; i < NUM_OF_NPC; ++i) {
		g_clients[i].is_use = true;
		g_clients[i].x = rand() % BOARD_WIDTH;
		g_clients[i].y = rand() % BOARD_HEIGHT;

		g_clients[i].zone_x = g_clients[i].x / ZONE_INTERVAL;
		g_clients[i].zone_y = g_clients[i].y / ZONE_INTERVAL;

		g_zone[g_clients[i].zone_y][g_clients[i].zone_x].insert(i);
	}

	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
}

bool Server::CanSee(int cl1, int cl2) {
	int dist_sq = (g_clients[cl1].x - g_clients[cl2].x)*(g_clients[cl1].x - g_clients[cl2].x) +
		(g_clients[cl1].y - g_clients[cl2].y)*(g_clients[cl1].y - g_clients[cl2].y);
	return (dist_sq <= VIEW_RADIUS * VIEW_RADIUS);
}

void Server::addViewList(unordered_set<int>& viewList, const int clientID, const int x, const int y) {
	//�ش� ���� �ִ� Ŭ���̾�Ʈ���� viewList�� �μ�Ʈ
	for (auto& i : g_zone[y][x]) {
		if (i == clientID) continue;
		if (!g_clients[i].is_use) continue;

		if (!CanSee(clientID, i)) continue;
		viewList.insert(i);
	}
}

void Server::SendPacket(int id, void* packet) {
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
		if (WSA_IO_PENDING != err_no) //�����ڵ尡 �̰��̸� ������ �ƴϴ�. (send�� �����µ� ��� send�� ���)
			err_display("Error in SendPacket : ", err_no);
	}

	cout << "SendPacket to Client [" << id << "] type ["
		<< (int)p[1] << "] size [" << (int)p[0] << "]\n";
}

void Server::SendPutObject(int client, int object) {
	sc_packet_put_player p;
	p.id = object;
	p.size = sizeof(p);
	p.type = SC_PUT_PLAYER;
	p.x = g_clients[object].x;
	p.y = g_clients[object].y;

	SendPacket(client, &p);
}

void Server::SendRemoveObject(int client, int object) {
	sc_packet_remove_player p;
	p.id = object;
	p.size = sizeof(p);
	p.type = SC_REMOVE_PLAYER;

	SendPacket(client, &p);
}

void Server::DisconnectPlayer(int id) {
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

void Server::ProcessPacket(int clientID, char* packet) {
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

	g_clients[clientID].zone_x = g_clients[clientID].x / ZONE_INTERVAL;
	g_clients[clientID].zone_y = g_clients[clientID].y / ZONE_INTERVAL;

	if (g_clients[clientID].zone_x != prev_x || g_clients[clientID].zone_y != prev_y) {
		g_zone[prev_y][prev_x].erase(clientID);
		g_zone[g_clients[clientID].zone_y][g_clients[clientID].zone_x].insert(clientID);
	}

	//npc�� view����Ʈ�� ����, sendPacket�� �� �ʿ䵵 ����.
	//�׷��Ƿ� �ٷ� ����
	if (clientID >= NPC_START)
		return;
	sc_packet_pos posPacket;
	posPacket.id = clientID;
	posPacket.size = sizeof(sc_packet_pos);
	posPacket.type = SC_POS;
	posPacket.x = g_clients[clientID].x;
	posPacket.y = g_clients[clientID].y;

	unordered_set<int> new_viewList;


	//���� ������
	addViewList(new_viewList, clientID, g_clients[clientID].zone_x, g_clients[clientID].zone_y);
	UINT x_interval = g_clients[clientID].x % ZONE_INTERVAL;
	UINT y_interval = g_clients[clientID].y % ZONE_INTERVAL;

	//�� ���� �� ó��
	if (x_interval < 3 || y_interval < 3) {
		if (x_interval < 3 && y_interval < 3) {
			if (g_clients[clientID].zone_y > 0 && g_clients[clientID].zone_x > 0) {
				addViewList(new_viewList, clientID, g_clients[clientID].zone_x - 1, g_clients[clientID].zone_y - 1);
			}
		}
		if (x_interval < 3) {
			if (g_clients[clientID].zone_x > 0) {
				addViewList(new_viewList, clientID, g_clients[clientID].zone_x - 1, g_clients[clientID].zone_y);
			}
		}
		if (y_interval < 3) {
			if (g_clients[clientID].zone_y > 0) {
				addViewList(new_viewList, clientID, g_clients[clientID].zone_x, g_clients[clientID].zone_y - 1);
			}
		}
	}
	else {
		if (x_interval > 17 && y_interval > 17) {
			if (g_clients[clientID].zone_y < ZONE_INTERVAL - 1 && g_clients[clientID].zone_x < ZONE_INTERVAL - 1) {
				addViewList(new_viewList, clientID, g_clients[clientID].zone_x + 1, g_clients[clientID].zone_y + 1);
			}
		}
		if (x_interval > 17) {
			if (g_clients[clientID].zone_x < ZONE_INTERVAL - 1) {
				addViewList(new_viewList, clientID, g_clients[clientID].zone_x + 1, g_clients[clientID].zone_y);
			}
		}
		if (y_interval > 17) {
			if (g_clients[clientID].zone_y < ZONE_INTERVAL - 1) {
				addViewList(new_viewList, clientID, g_clients[clientID].zone_x, g_clients[clientID].zone_y + 1);
			}
		}
	}


	for (auto& id : new_viewList) {
		//���� viewlist�� ������ ��ü ó��
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
			//view�� ��� �����ִ� ��ü ó��
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
	//viewlist���� ������ ��ü ó��
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

void Server::AcceptNewClient(SOCKET& g_socket)
{
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
		return;
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

	g_clients[new_key].zone_x = p.x / ZONE_INTERVAL;
	g_clients[new_key].zone_y = p.y / ZONE_INTERVAL;

	g_zone[g_clients[new_key].zone_y][g_clients[new_key].zone_x].insert(new_key);

	//���� ������ �ٸ� �÷��̾�� �˸� (���� ����)
	//���� ���� �ִ� �÷��̾�� �˸�
	for (auto& i : g_zone[g_clients[new_key].zone_y][g_clients[new_key].zone_x]) {
		if (i >= MAX_USER) //user�� �ƴϸ� ����
			continue;
		if (g_clients[i].is_use) {
			if (!Server::getInstance()->CanSee(i, new_key))
				continue;
			g_clients[i].vlm.lock();
			if (i != new_key)
				g_clients[i].viewlist.insert(new_key);
			g_clients[i].vlm.unlock();
			Server::getInstance()->SendPacket(i, &p);
		}
	}

	//������ �������� �ٸ� �÷��̾��� ������ ����
	//(NPC, �÷��̾� ����)
	for (auto& i : g_zone[g_clients[new_key].zone_y][g_clients[new_key].zone_x]) {
		if (g_clients[i].is_use) {
			if (i == new_key)
				continue;
			if (!Server::getInstance()->CanSee(i, new_key))
				continue;
			p.id = i;
			p.x = g_clients[i].x;
			p.y = g_clients[i].y;
			g_clients[new_key].vlm.lock();
			g_clients[new_key].viewlist.insert(i);
			g_clients[new_key].vlm.unlock();
			Server::getInstance()->SendPacket(new_key, &p);
		}
	}
}

Client * Server::getClient(int id)
{
	return &g_clients[id];
}

HANDLE * Server::getIOCP()
{
	return &g_iocp;
}

void Server::recv(unsigned long long& key, unsigned long& data_size, EXOver* exover)
{
	int recv_size = data_size;
	char* ptr = exover->io_buf;
	while (recv_size > 0) {
		if (g_clients[key].packet_size == 0)
			g_clients[key].packet_size = ptr[0]; //��Ŷ�� ������� �� ��
												 //�� ó�� recv�� �ϰų�, ��� ó���ؼ� ���ο� ��Ŷ�� �޾ƾ� �ϴ� ���

		int remain = g_clients[key].packet_size - g_clients[key].prev_size;
		if (remain <= recv_size) {
			//build packet
			memcpy(g_clients[key].prev_packet + g_clients[key].prev_size,
				ptr, remain);
			//��Ŷ ó��
			Server::getInstance()->ProcessPacket(static_cast<int>(key), g_clients[key].prev_packet);
			recv_size -= remain;
			ptr += remain;
			g_clients[key].packet_size = 0;
			g_clients[key].prev_size = 0;
		}
		else {
			//���� ��� ��Ŷ�� ���� ���� ��� ������ ����
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