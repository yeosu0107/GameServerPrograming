#include "stdafx.h"
#include "Server.h"


Server* Server::g_server = nullptr;

bool Server::isNPC(int index)
{
	if (index >= NPC_START)
		return true;
	return false;
}

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
	//g_clients.resize(NUM_OF_NPC, new Client());
	for (int i = 0; i < NPC_START; ++i) {
		Client* client = new Client();
		g_clients.emplace_back(client);
	}
	wcout.imbue(locale("korean"));

	for (int i = NPC_START; i < NUM_OF_NPC; ++i) {
		Npc* now = new Npc();
		now->is_use = true;
		now->is_active = false;
		now->x = rand() % BOARD_WIDTH;
		now->y = rand() % BOARD_HEIGHT;

		now->zone_x = now->x / ZONE_INTERVAL;
		now->zone_y = now->y / ZONE_INTERVAL;

		g_zone[now->zone_y][now->zone_x].insert(i);
		g_clients.emplace_back(now);
	}
	
	//MoveNpc();

	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
}

bool Server::CanSee(int cl1, int cl2) {
	Client* from = reinterpret_cast<Client*>(g_clients[cl1]);
	Client* target = reinterpret_cast<Client*>(g_clients[cl2]);
	int dist_sq = (from->x - target->x)*(from->x - target->x) +
		(from->y - target->y)*(from->y - target->y);
	return (dist_sq <= VIEW_RADIUS * VIEW_RADIUS);
}

void Server::addViewList(unordered_set<int>& viewList, const int clientID, const int x, const int y) {
	//해당 존에 있는 클라이언트들을 viewList에 인서트
	if (!isNPC(clientID)) {
		for (auto& i : g_zone[y][x]) {
			Client* now = reinterpret_cast<Client*>(g_clients[i]);
			if (i == clientID) continue;
			if (!now->is_use) continue;
			if (!CanSee(clientID, i)) continue;
			viewList.insert(i);
		}
	}
	else {
		for (auto& i : g_zone[y][x]) {
			Client* now = reinterpret_cast<Client*>(g_clients[i]);
			if (i == clientID) continue;
			if (isNPC(i)) continue;
			if (!now->is_use) continue;
			//if (!CanSee(clientID, i)) continue;
			viewList.insert(i);
		}
	}
}

void Server::SendPacket(int id, void* packet) {
	EXOver* over = new EXOver;
	char* p = reinterpret_cast<char*>(packet);
	memcpy(over->io_buf, p, p[0]);
	over->event_type = EV_SEND;
	over->wsabuf.buf = over->io_buf;
	over->wsabuf.len = p[0];

	ZeroMemory(&over->wsaover, sizeof(WSAOVERLAPPED));

	int ret = WSASend(reinterpret_cast<Client*>(g_clients[id])->s, 
		&over->wsabuf, 1, NULL, 0, &over->wsaover, NULL);

	if (ret != 0) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no) //에러코드가 이것이면 에러가 아니다. (send가 끝났는데 계속 send인 경우)
			err_display("Error in SendPacket : ", err_no);
	}
}

void Server::SendPutObject(int client, int object) {
	sc_packet_put_player p;
	p.id = object;
	p.size = sizeof(p);
	p.type = SC_PUT_PLAYER;
	p.x = g_clients[object]->x;
	p.y = g_clients[object]->y;

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
	Client* now = reinterpret_cast<Client*>(g_clients[id]);

	closesocket(now->s);

	cout << "Client [" << id << "] DisConnected\n";

	sc_packet_remove_player p;
	p.id = id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_PLAYER;

	now->vlm.lock();
	unordered_set<int> vl_copy = now->viewlist;
	now->viewlist.clear();
	now->vlm.unlock();
	for (auto& i : vl_copy) {
		if (isNPC(i)) continue;
		Client* target = reinterpret_cast<Client*>(g_clients[i]);
		target->vlm.lock();
		if (target->is_use) {
			if (target->viewlist.count(id) != 0) {
				target->viewlist.erase(id);
				target->vlm.unlock();
				SendPacket(i, &p);
			}
		}
		else {
			target->vlm.unlock();
		}
	}
	now->is_use = false;
}

void Server::ProcessPacket(int clientID, char* packet) {
	cs_packet_up* p = reinterpret_cast<cs_packet_up*>(packet);
	Client* client = reinterpret_cast<Client*>(g_clients[clientID]);

	switch (p->type) {
	case CS_UP:
		client->y -= 1;
		if (client->y < 0)
			client->y = 0;
		break;
	case CS_DOWN:
		client->y += 1;
		if (client->y >= BOARD_HEIGHT)
			client->y = BOARD_HEIGHT - 1;
		break;
	case CS_LEFT:
		client->x -= 1;
		if (client->x < 0)
			client->x = 0;
		break;
	case CS_RIGHT:
		client->x += 1;
		if (client->x >= BOARD_WIDTH)
			client->x = BOARD_WIDTH - 1;
		break;
	default:
		cout << "Unknown Protocol from Client [" << clientID << "]\n";
		return;
	}
	int prev_x = client->zone_x;
	int prev_y = client->zone_y;

	client->zone_x = client->x / ZONE_INTERVAL;
	client->zone_y = client->y / ZONE_INTERVAL;

	if (client->zone_x != prev_x || client->zone_y != prev_y) {
		g_zone[prev_y][prev_x].erase(clientID);
		g_zone[client->zone_y][client->zone_x].insert(clientID);
	}

	client->type = 0;
	sc_packet_pos posPacket;
	posPacket.id = clientID;
	posPacket.size = sizeof(sc_packet_pos);
	posPacket.type = SC_POS;
	posPacket.x = client->x;
	posPacket.y = client->y;

	unordered_set<int> new_viewList;

	//나와 같은존
	addViewList(new_viewList, clientID, client->zone_x, client->zone_y);
	UINT x_interval = client->x % ZONE_INTERVAL;
	UINT y_interval = client->y % ZONE_INTERVAL;

	//내 인접 존 처리
	if (x_interval < 7 && y_interval < 7) {
		if (client->zone_y > 0 && client->zone_x > 0) {
			addViewList(new_viewList, clientID, client->zone_x - 1, client->zone_y - 1);
		}
	}
	else if (x_interval > 13 && y_interval > 13) {
		if (client->zone_y < ZONE_INTERVAL - 1 && client->zone_x < ZONE_INTERVAL - 1) {
			addViewList(new_viewList, clientID, client->zone_x + 1, client->zone_y + 1);
		}
	}
	if (x_interval < 7) {
		if (client->zone_x > 0) {
			addViewList(new_viewList, clientID, client->zone_x - 1, client->zone_y);
		}
	}
	else if (x_interval > 13) {
		if (client->zone_x < ZONE_INTERVAL - 1) {
			addViewList(new_viewList, clientID, client->zone_x + 1, client->zone_y);
		}
	}
	if (y_interval < 7) {
		if (client->zone_y > 0) {
			addViewList(new_viewList, clientID, client->zone_x, client->zone_y - 1);
		}
	}
	else if (y_interval > 13) {
		if (client->zone_y < ZONE_INTERVAL - 1) {
			addViewList(new_viewList, clientID, client->zone_x, client->zone_y + 1);
		}
	}


	for (auto& id : new_viewList) {
		//새로 viewlist에 들어오는 객체 처리
		client->vlm.lock();
		if (client->viewlist.count(id) == 0) {
			client->viewlist.insert(id);
			WakeUpNPC(id);
			client->vlm.unlock();
			SendPutObject(clientID, id);

			if (isNPC(id)) continue;

			Client* target = reinterpret_cast<Client*>(g_clients[id]);
			target->vlm.lock();
			if (target->viewlist.count(clientID) == 0) {
				target->viewlist.insert(clientID);
				target->vlm.unlock();
				SendPutObject(id, clientID);
			}
			else {
				target->vlm.unlock();
				SendPacket(id, &posPacket);
			}
		}
		else {
			client->vlm.unlock();
			//view에 계속 남아있는 객체 처리
			if (isNPC(id)) continue;

			Client* target = reinterpret_cast<Client*>(g_clients[id]);
			target->vlm.lock();
			if (target->viewlist.count(clientID) == 0) {
				target->viewlist.insert(clientID);
				target->vlm.unlock();
				SendPutObject(id, clientID);
			}
			else {
				target->vlm.unlock();
				SendPacket(id, &posPacket);
			}
		}
	}

	//viewlist에서 나가는 객체 처리
	vector<int> tmpDeleteList;
	client->vlm.lock();
	unordered_set<int> old_vl = client->viewlist;
	client->vlm.unlock();
	for (auto& id : old_vl) {
		if (0 == new_viewList.count(id)) {
			tmpDeleteList.emplace_back(id);

			if (isNPC(id)) continue;

			Client* target = reinterpret_cast<Client*>(g_clients[id]);
			target->vlm.lock();
			if (0 != target->viewlist.count(clientID)) {
				target->viewlist.erase(clientID);
				target->vlm.unlock();
				SendRemoveObject(id, clientID);
			}
			else
				target->vlm.unlock();

		}
	}


	for (auto& delid : tmpDeleteList) {
		client->vlm.lock();
		client->viewlist.erase(delid);
		client->vlm.unlock();
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
		Client* now = reinterpret_cast<Client*>(g_clients[i]);
		if (!now->is_use) {
			new_key = i;
			break;
		}
	}
	if (new_key == -1) {
		cout << "MAX USER EXCEEDED!!\n";
		return;
	}
	cout << "New Client's ID : " << new_key << endl;
	Client* newClient = reinterpret_cast<Client*>(g_clients[new_key]);

	newClient->s = new_socket;
	newClient->x = 10;
	newClient->y = 10;
	ZeroMemory(&newClient->exover.wsaover, sizeof(WSAOVERLAPPED));

	CreateIoCompletionPort(reinterpret_cast<HANDLE>(new_socket),
		g_iocp, new_key, 0);

	newClient->viewlist.clear();
	newClient->is_use = true;

	unsigned long flag = 0;
	int ret = WSARecv(new_socket, &newClient->exover.wsabuf, 1, NULL, &flag,
		&newClient->exover.wsaover, NULL);

	if (ret != 0) {
		int err_no = WSAGetLastError();
		if (err_no != WSA_IO_PENDING)
			err_display("Recv in AcceptThread", err_no);
	}

	sc_packet_put_player p;
	p.id = new_key;
	p.size = sizeof(sc_packet_put_player);
	p.type = SC_PUT_PLAYER;
	p.x = newClient->x;
	p.y = newClient->y;

	newClient->zone_x = p.x / ZONE_INTERVAL;
	newClient->zone_y = p.y / ZONE_INTERVAL;

	g_zone[newClient->zone_y][newClient->zone_x].insert(new_key);
	//나의 접속을 다른 플레이어에게 알림 (나를 포함)
	//같은 존에 있는 플레이어에만 알림
	for (auto& i : g_zone[newClient->zone_y][newClient->zone_x]) {
		if (isNPC(i)) continue;
		Client* other = reinterpret_cast<Client*>(g_clients[i]);
		if (other->is_use) {
			if (!Server::getInstance()->CanSee(i, new_key))
				continue;
			other->vlm.lock();
			if (i != new_key)
				other->viewlist.insert(new_key);
			other->vlm.unlock();
			Server::getInstance()->SendPacket(i, &p);
		}
	}

	//나에게 접속중인 다른 플레이어의 정보를 전송
	//(NPC, 플레이어 포함)
	for (auto& i : g_zone[newClient->zone_y][newClient->zone_x]) {
		if (g_clients[i]->is_use) {
			if (i == new_key)
				continue;
			if (!Server::getInstance()->CanSee(i, new_key))
				continue;
			p.id = i;
			p.x = g_clients[i]->x;
			p.y = g_clients[i]->y;
			newClient->vlm.lock();
			newClient->viewlist.insert(i);
			newClient->vlm.unlock();
			WakeUpNPC(i);
			Server::getInstance()->SendPacket(new_key, &p);
		}
	}
}

void Server::add_timer(int id, int type, float time)
{
	Event now;
	now.id = id;
	now.time = time;
	now.startClock = chrono::system_clock::now();
	now.type = type;

	tmp.lock();
	event_queue.push(now);
	tmp.unlock();
}

void Server::MoveNpc(int key)
{
	switch (rand()%4 + 1) {
	case CS_UP:
		g_clients[key]->y -= 1;
		if (g_clients[key]->y < 0)
			g_clients[key]->y = 0;
		break;
	case CS_DOWN:
		g_clients[key]->y += 1;
		if (g_clients[key]->y >= BOARD_HEIGHT)
			g_clients[key]->y = BOARD_HEIGHT - 1;
		break;
	case CS_LEFT:
		g_clients[key]->x -= 1;
		if (g_clients[key]->x < 0)
			g_clients[key]->x = 0;
		break;
	case CS_RIGHT:
		g_clients[key]->x += 1;
		if (g_clients[key]->x >= BOARD_WIDTH)
			g_clients[key]->x = BOARD_WIDTH - 1;
		break;
	default:
		return;
	}
	sc_packet_pos posPacket;
	posPacket.id = key;
	posPacket.size = sizeof(sc_packet_pos);
	posPacket.type = SC_POS;
	posPacket.x = g_clients[key]->x;
	posPacket.y = g_clients[key]->y;

	unordered_set<int> new_viewList;
	//같은존
	addViewList(new_viewList, key, g_clients[key]->zone_x, g_clients[key]->zone_y);
	UINT x_interval = g_clients[key]->x % ZONE_INTERVAL;
	UINT y_interval = g_clients[key]->y % ZONE_INTERVAL;

	//인접 존 처리
	if (x_interval < 7 && y_interval < 7) {
		if (g_clients[key]->zone_y > 0 && g_clients[key]->zone_x > 0) {
			addViewList(new_viewList, key, g_clients[key]->zone_x - 1, g_clients[key]->zone_y - 1);
		}
	}
	else if (x_interval > 13 && y_interval > 13) {
		if (g_clients[key]->zone_y < ZONE_INTERVAL - 1 && g_clients[key]->zone_x < ZONE_INTERVAL - 1) {
			addViewList(new_viewList, key, g_clients[key]->zone_x + 1, g_clients[key]->zone_y + 1);
		}
	}
	if (x_interval < 7) {
		if (g_clients[key]->zone_x > 0) {
			addViewList(new_viewList, key, g_clients[key]->zone_x - 1, g_clients[key]->zone_y);
		}
	}
	else if (x_interval > 13) {
		if (g_clients[key]->zone_x < ZONE_INTERVAL - 1) {
			addViewList(new_viewList, key, g_clients[key]->zone_x + 1, g_clients[key]->zone_y);
		}
	}
	if (y_interval < 7) {
		if (g_clients[key]->zone_y > 0) {
			addViewList(new_viewList, key, g_clients[key]->zone_x, g_clients[key]->zone_y - 1);
		}
	}
	else if (y_interval > 13) {
		if (g_clients[key]->zone_y < ZONE_INTERVAL - 1) {
			addViewList(new_viewList, key, g_clients[key]->zone_x, g_clients[key]->zone_y + 1);
		}
	}
	for (auto& id : new_viewList) {
		Client* target = reinterpret_cast<Client*>(g_clients[id]);
		if (CanSee(key, id)) {
			target->vlm.lock();
			if (target->viewlist.count(key) == 0) {
				target->viewlist.insert(key);
				target->vlm.unlock();
				SendPutObject(id, key);
			}
			else {
				target->vlm.unlock();
				SendPacket(id, &posPacket);
			}
		}
		else {
			if (target->viewlist.count(key) != 0) {
				target->vlm.lock();
				target->viewlist.erase(key);
				target->vlm.unlock();
				SendRemoveObject(id, key);
			}
		}
	}

	if (!new_viewList.empty()) {
		add_timer(key, MOVE_TYPE, 1);
	}
	else {
		g_clients[key]->is_active = false;
	}
}

void Server::WakeUpNPC(int id)
{
	if (!isNPC(id))
		return;
	if (g_clients[id]->is_active)
		return;
	
	g_clients[id]->is_active = true; //CAS(&is_active, false, true)
	add_timer(id, MOVE_TYPE, 1.0f);
	
}

Object * Server::getClient(int id)
{
	return g_clients[id];
}

HANDLE * Server::getIOCP()
{
	return &g_iocp;
}

void Server::recv(unsigned long long& key, unsigned long& data_size, EXOver* exover)
{
	int recv_size = data_size;
	char* ptr = exover->io_buf;

	if (!isNPC(key)) {
		Client* client = reinterpret_cast<Client*>(g_clients[key]);
		while (recv_size > 0) {
			if (client->packet_size == 0)
				client->packet_size = ptr[0]; //패킷의 사이즈는 맨 앞
													 //맨 처음 recv를 하거나, 모두 처리해서 새로운 패킷을 받아야 하는 경우

			int remain = client->packet_size - client->prev_size;
			if (remain <= recv_size) {
				//build packet
				memcpy(client->prev_packet + client->prev_size,
					ptr, remain);
				//패킷 처리
				ProcessPacket(static_cast<int>(key), client->prev_packet);
				recv_size -= remain;
				ptr += remain;
				client->packet_size = 0;
				client->prev_size = 0;
			}
			else {
				//아직 모든 패킷을 받지 않은 경우 재조립 수행
				memcpy(client->prev_packet + client->prev_size,
					ptr, recv_size);
				client->prev_size += recv_size;
				recv_size -= recv_size;
				ptr += recv_size;
			}
		}
		unsigned long rflag = 0;
		ZeroMemory(&exover->wsaover, sizeof(WSAOVERLAPPED));
		int ret = WSARecv(client->s, &exover->wsabuf, 1, NULL,
			&rflag, &exover->wsaover, NULL);

		if (ret != 0) {
			int err_no = WSAGetLastError();
			if (err_no != WSA_IO_PENDING)
				err_display("Recv in WorkThread", err_no);
		}
	}
	else {
		ProcessPacket(static_cast<int>(key), exover->io_buf);
	}
}
