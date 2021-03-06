#include <string>
#include "stdafx.h"
#include "Server.h"



Server* Server::g_server = nullptr;

Quest quest1(0, 1);
Quest quest2(1, 1);
Quest quest3(2, 1);
Quest quest4(3, 1);
Quest quest5(4, 10);
Quest quest6(5, 20);

Quest quest[6] = { quest1, quest2, quest3, quest4, quest5, quest6 };

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
	CsvMap mapFile;
	g_collisionMap = mapFile.getCollisionMap();
	g_spawnPoint = mapFile.getSpawnPoint();
	g_expTable = mapFile.getExpTable();
	g_MaxLevel = g_expTable.size();

	//g_clients.resize(NUM_OF_NPC, new Client());
	wcout.imbue(locale("korean"));
	for (int i = 0; i < NPC_START; ++i) {
		Client* client = new Client();
		g_clients.emplace_back(client);
	}
	cout << "Player Info Initialize Complete" << endl;
	srand((unsigned)time(NULL));
	for (int i = NPC_START; i < NUM_OF_NPC; ++i) {
		spawnPoint info = g_spawnPoint.front();
		g_spawnPoint.pop();

		int xPos = info.xPos;
		int yPos = info.yPos;

		g_zone[yPos / ZONE_INTERVAL][xPos/ZONE_INTERVAL].emplace(i);

		ScriptEngine* ai = new ScriptEngine("lua_script\\monster.lua", i);
		lua_register(ai->getInstance(), "API_get_x", CAPI_getX);
		lua_register(ai->getInstance(), "API_get_y", CAPI_getY);
		lua_register(ai->getInstance(), "API_send_msg", CAPI_sendMsg);
		lua_register(ai->getInstance(), "API_npc_move", CAPI_moveNPC);
		g_clients.emplace_back(new Npc(xPos, yPos, ai, info.type));
		if (info.type == MONSTER_BOSS) {
			Npc* boss = reinterpret_cast<Npc*>(g_clients[i]);
			lua_getglobal(boss->aiScript->getInstance(), "set_dist");
			lua_pushnumber(boss->aiScript->getInstance(), 10);
			lua_pcall(boss->aiScript->getInstance(), 1, 0, 0);
		}
	}
	cout << "Npc Info Initialize Complete" << endl;

	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
#ifdef DB
	UploadUserDatatoDB();
#endif
	cout << "Server Initialize Success" << endl;
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
		g_mutex[y][x].lock();
		unordered_set<int> tmpZone = g_zone[y][x];
		g_mutex[y][x].unlock();
		for (const int& i : tmpZone) {
			if (i == clientID) continue;
			if (!g_clients[i]->is_use) continue;
			if (!CanSee(clientID, i)) continue;
			viewList.emplace(i);

			if (isNPC(i)) {
				EXOver* exover = new EXOver;
				exover->event_type = EV_PLAYER_MOVE;
				exover->event_target = clientID;
				PostQueuedCompletionStatus(g_iocp, 1, i, &exover->wsaover);
			}
		}
	}
	else {
		g_mutex[y][x].lock();
		unordered_set<int> tmpZone = g_zone[y][x];
		g_mutex[y][x].unlock();
		for (const int& i : tmpZone) {
			if (i == clientID) continue;
			if (isNPC(i)) continue;
			if (!g_clients[i]->is_use) continue;
			if (!CanSee(clientID, i)) continue;
			viewList.emplace(i);
		}
	}
}

unordered_set<int> Server::ProcessNearZone(int key)
{
	unordered_set<int> new_viewList;
	//같은존
	addViewList(new_viewList, key, g_clients[key]->zone_x, g_clients[key]->zone_y);
	
	if(isNPC(key))
		return new_viewList;

	UINT x_interval = g_clients[key]->x % ZONE_INTERVAL;
	UINT y_interval = g_clients[key]->y % ZONE_INTERVAL;

	////인접 존 처리
	if (x_interval < VIEW_RADIUS && y_interval < VIEW_RADIUS) {
		if (g_clients[key]->zone_y > 0 && g_clients[key]->zone_x > 0) {
			addViewList(new_viewList, key, g_clients[key]->zone_x - 1, g_clients[key]->zone_y - 1);
		}
	}
	else if (x_interval > ZONE_EDGH && y_interval > ZONE_EDGH) {
		if (g_clients[key]->zone_y < ZONE_INTERVAL - 1 && g_clients[key]->zone_x < ZONE_INTERVAL - 1) {
			addViewList(new_viewList, key, g_clients[key]->zone_x + 1, g_clients[key]->zone_y + 1);
		}
	}
	if (x_interval < VIEW_RADIUS) {
		if (g_clients[key]->zone_x > 0) {
			addViewList(new_viewList, key, g_clients[key]->zone_x - 1, g_clients[key]->zone_y);
		}
	}
	else if (x_interval > ZONE_EDGH) {
		if (g_clients[key]->zone_x < ZONE_INTERVAL - 1) {
			addViewList(new_viewList, key, g_clients[key]->zone_x + 1, g_clients[key]->zone_y);
		}
	}
	if (y_interval < VIEW_RADIUS) {
		if (g_clients[key]->zone_y > 0) {
			addViewList(new_viewList, key, g_clients[key]->zone_x, g_clients[key]->zone_y - 1);
		}
	}
	else if (y_interval > ZONE_EDGH) {
		if (g_clients[key]->zone_y < ZONE_INTERVAL - 1) {
			addViewList(new_viewList, key, g_clients[key]->zone_x, g_clients[key]->zone_y + 1);
		}
	}

	return new_viewList;
}

bool Server::checkCollisionMap(int index)
{
	if (g_collisionMap[g_clients[index]->y][g_clients[index]->x] != -1)
		return true;
	return false;
}

bool Server::checkCollisionMap(int x, int y)
{
	if (g_collisionMap[y][x] != -1)
		return true;
	return false;
}

void Server::SendPacket(int id, void* packet) {
	EXOver* over = new EXOver;
	unsigned char* p = reinterpret_cast<unsigned char*>(packet);
	memcpy(over->io_buf, p, p[0]);
	over->event_type = EV_SEND;
	over->wsabuf.buf = over->io_buf;
	over->wsabuf.len = p[0];

	ZeroMemory(&over->wsaover, sizeof(WSAOVERLAPPED));

	int ret = WSASend(reinterpret_cast<Client*>(g_clients[id])->s, 
		&over->wsabuf, 1, NULL, 0, &over->wsaover, NULL);

	if (ret != 0) {
		int err_no = WSAGetLastError();
		//if (WSA_IO_PENDING != err_no) //에러코드가 이것이면 에러가 아니다. (send가 끝났는데 계속 send인 경우)
		//	err_display("Error in SendPacket : ", err_no);
	}
}

void Server::SendPutObject(int client, int object) {
	sc_packet_put_player p;
	p.id = object;
	p.size = sizeof(p);
	p.type = SC_PUT_PLAYER;
	p.x = g_clients[object]->x;
	p.y = g_clients[object]->y;

	if (object >= NPC_START)
		p.pic_type = reinterpret_cast<Npc*>(g_clients[object])->type;
	else
		p.pic_type = -1;

	SendPacket(client, &p);
}

void Server::SendRemoveObject(int client, int object) {
	sc_packet_remove_player p;
	p.id = object;
	p.size = sizeof(p);
	p.type = SC_REMOVE_PLAYER;

	SendPacket(client, &p);
}

void Server::SendChatPacket(int to, int from, const wchar_t * msg, int info)
{
	sc_packet_chat p;
	p.id = from;
	p.size = sizeof(p);
	p.type = SC_CHAT;
	p.info = info;
	wcscpy_s(p.message, msg);

	SendPacket(to, &p);
}

void Server::SendStatPacket(int id)
{
	Client* client = reinterpret_cast<Client*>(g_clients[id]);

	sc_packet_stat p;
	p.size = sizeof(p);
	p.type = SC_PLAYER_STAT;
	p.exp = client->exp;
	p.hp = client->hp;
	p.level = client->level;
	p.max_exp = g_expTable[client->level];

	SendPacket(id, &p);
}

void Server::SendSkillPacket(int id, int kind)
{
	sc_packet_skill p;
	p.size = sizeof(p);
	p.type = SC_PLAYER_SKILL;
	p.kind = kind;

	SendPacket(id, &p);
}

void Server::DisconnectPlayer(int id) {
	Client* now = reinterpret_cast<Client*>(g_clients[id]);

	closesocket(now->s);

	//cout << "Client [" << id << "] DisConnected\n";
#ifdef DB
	//DBEvent newEvent = DBEvent(UPDATE_POS, now->player_id, now, id);
	//db_queue.push(newEvent);
	db_queue.emplace(DBEvent(UPDATE_POS, now->player_id, now, id));
	UpdateUserInfotoDB(id);
#endif
	sc_packet_remove_player p;
	p.id = id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_PLAYER;

	now->vlm.lock();
	unordered_set<int> vl_copy = now->viewlist;
	now->viewlist.clear();
	unordered_set<int> del;
	now->viewlist.swap(del);
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
	now->player_id = -1;
}
int randx[15] = { 129, 80,177,129,129, 14, 14, 57, 189, 160, 32, 99, 85, 33, 87 };
int randy[15] = { 163,191, 191,218,190, 76, 222, 232, 231, 134, 139, 112, 150, 191, 89 };
void Server::ProcessPacket(int clientID, char* packet) {
	cs_packet_up* p = reinterpret_cast<cs_packet_up*>(packet);
	Client* client = reinterpret_cast<Client*>(g_clients[clientID]);

	if (client->stun == true) {
		DWORD remain = GetTickCount() - client->stunTime;
		if (remain < 2000) {
			SendChatPacket(clientID, clientID, L"You're stunned!", INFO_NONE);
			return;
		}
		else
			client->stun = false;
	}

	int origin_x = client->x;
	int origin_y = client->y;
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
	case CS_RANDOM:
	{
		int index = rand() % 15;
		client->x = randx[index];
		client->y = randy[index];
	}
		break;
	case CS_ATTACK:
		CheckQuest(client, clientID);
		PlayerAttack(clientID);
		//printf("recv attack message : player %d\n", clientID);
		return;
	case CS_SKILL1:
		PlayerSkill(clientID);
		return;
	case CS_SKILL2:
		PlayerSkill2(clientID);
		//printf("press skill2 : player %d\n", clientID);
		return;
	case CS_GOTOWN:
		client->x = 129;
		client->y = 191;
		break;
	default:
		//cout << "Unknown Protocol from Client [" << clientID << "]\n";
		return;
	}
	if (p->type < CS_ATTACK) {
		if (!CheckPlayerLevel(client)) {
			wchar_t tmp[21] = L"Can't go. Be Strong!";
			SendChatPacket(clientID, clientID, tmp, INFO_ATTACK);
			client->x = origin_x;
			client->y = origin_y;
			return;
		}
		if (checkCollisionMap(clientID)) {
			client->x = origin_x;
			client->y = origin_y;
			return;
		}
		int prev_x = client->zone_x;
		int prev_y = client->zone_y;

		client->zone_x = client->x / ZONE_INTERVAL;
		client->zone_y = client->y / ZONE_INTERVAL;

		if (client->zone_x != prev_x || client->zone_y != prev_y) {
			g_zone[prev_y][prev_x].erase(clientID);
			g_zone[client->zone_y][client->zone_x].emplace(clientID);
		}

		client->type = 0;
		sc_packet_pos posPacket;
		posPacket.id = clientID;
		posPacket.size = sizeof(sc_packet_pos);
		posPacket.type = SC_POS;
		posPacket.x = client->x;
		posPacket.y = client->y;

		unordered_set<int> new_viewList;// = ProcessNearZone(clientID);
		new_viewList.swap(*&ProcessNearZone(clientID));

		for (auto& id : new_viewList) {
			//새로 viewlist에 들어오는 객체 처리
			client->vlm.lock();
			if (client->viewlist.count(id) == 0) {
				client->viewlist.emplace(id);
				//WakeUpNPC(id);
				client->vlm.unlock();
				SendPutObject(clientID, id);

				//if (isNPC(id)) continue;

				Npc* target = reinterpret_cast<Npc*>(g_clients[id]);
				target->vlm.lock();
				if (target->viewlist.count(clientID) == 0) {
					target->viewlist.emplace(clientID);
					target->vlm.unlock();
					if (isNPC(id)) continue;
					SendPutObject(id, clientID);
				}
				else {
					target->vlm.unlock();
					if (isNPC(id)) continue;
					SendPacket(id, &posPacket);
				}
			}
			else {
				client->vlm.unlock();
				//view에 계속 남아있는 객체 처리
				//if (isNPC(id)) continue;

				Npc* target = reinterpret_cast<Npc*>(g_clients[id]);
				target->vlm.lock();
				if (target->viewlist.count(clientID) == 0) {
					target->viewlist.emplace(clientID);
					target->vlm.unlock();
					if (isNPC(id)) continue;
					SendPutObject(id, clientID);
				}
				else {
					target->vlm.unlock();
					if (isNPC(id)) continue;
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

				//if (isNPC(id)) continue;

				Npc* target = reinterpret_cast<Npc*>(g_clients[id]);
				target->vlm.lock();
				if (0 != target->viewlist.count(clientID)) {
					target->viewlist.erase(clientID);
					target->vlm.unlock();
					if (isNPC(id)) continue;
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
}

void Server::AcceptAndSearchClient(SOCKET & g_socket)
{
	SOCKADDR_IN c_addr;
	ZeroMemory(&c_addr, sizeof(SOCKADDR_IN));
	c_addr.sin_family = AF_INET;
	c_addr.sin_port = htons(MY_SERVER_PORT);
	c_addr.sin_addr.s_addr = INADDR_ANY;
	int c_addr_len = sizeof(SOCKADDR_IN);

	auto new_socket = WSAAccept(g_socket, reinterpret_cast<SOCKADDR*>(&c_addr),
		&c_addr_len, NULL, NULL);
	//cout << "new Client Accepted!\n";

	int new_key = -1;

	for (int i = 0; i < MAX_USER; ++i) {
		//Client* now = reinterpret_cast<Client*>(g_clients[i]);
		if (!g_clients[i]->is_use) {
			new_key = i;
			break;
		}
	}
	if (new_key == -1) {
		cout << "MAX USER EXCEEDED!!\n";
		return;
	}
	//cout << "New Client's ID : " << new_key << endl;
	Client* newClient = reinterpret_cast<Client*>(g_clients[new_key]);

	newClient->s = new_socket;
	newClient->x = 129;
	newClient->y = 191;
	ZeroMemory(&newClient->exover.wsaover, sizeof(WSAOVERLAPPED));

	newClient->is_use = true;
	unordered_set<int> tmp;
	newClient->viewlist.swap(tmp);

#ifdef DB
	DWORD iobyte, ioflag = 0;
	DWORD in_packet_size = 0;
	int saved_packet_size = 0;
	WSABUF recv_wsabuf;

	char	recv_buffer[1024];
	char	packet_buffer[1024];

	recv_wsabuf.buf = recv_buffer;
	recv_wsabuf.len = 1024;

	int ret = WSARecv(new_socket, &recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);

	if (ret) {
		int err_code = WSAGetLastError();
		printf("Recv Error [%d]\n", err_code);
		closesocket(new_socket);
		newClient->is_use = false;
		return;
	}
	BYTE *ptr = reinterpret_cast<BYTE *>(recv_buffer);
	while (0 != iobyte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (iobyte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			
			SearchClientID(ptr, newClient, new_key);
			return;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, iobyte);
			saved_packet_size += iobyte;
			iobyte = 0;
		}
	}
#else
	SearchClientID(nullptr, newClient, new_key);
#endif
}

void Server::AcceptNewClient(Client* client, int new_key)
{
	Client* newClient = reinterpret_cast<Client*>(client);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(newClient->s),
		g_iocp, new_key, 0);
	printf("Accept %d\n", new_key);
	//newClient->viewlist.clear();
	//newClient->is_use = true;

	unsigned long flag = 0;
	int ret = WSARecv(newClient->s, &newClient->exover.wsabuf, 1, NULL, &flag,
		&newClient->exover.wsaover, NULL);

	if (ret != 0) {
		int err_no = WSAGetLastError();
		//if (err_no != WSA_IO_PENDING)
		//	err_display("Recv in AcceptThread", err_no);
	}

	sc_packet_put_player p;
	p.id = new_key;
	p.size = sizeof(sc_packet_put_player);
	p.type = SC_PUT_PLAYER;
	p.x = newClient->x;
	p.y = newClient->y;

	newClient->zone_x = p.x / ZONE_INTERVAL;
	newClient->zone_y = p.y / ZONE_INTERVAL;
	//g_mutex[newClient->zone_y][newClient->zone_x].lock();
	g_zone[newClient->zone_y][newClient->zone_x].emplace(new_key);
	//unordered_set<int> tmpList = g_zone[newClient->zone_y][newClient->zone_x];
	//g_mutex[newClient->zone_y][newClient->zone_x].unlock();
	unordered_set<int> nearList = ProcessNearZone(new_key);
	//나의 접속을 다른 플레이어에게 알림 (나를 포함)
	//같은 존 & 인접 존에 있는 플레이어에만 알림
	for (auto& i : g_zone[newClient->zone_y][newClient->zone_x]) {
		if (isNPC(i)) continue;
		Client* other = reinterpret_cast<Client*>(g_clients[i]);
		if (other->is_use) {
			if (!Server::getInstance()->CanSee(i, new_key))
				continue;
			other->vlm.lock();
			if (i != new_key)
				other->viewlist.emplace(new_key);
			other->vlm.unlock();
			Server::getInstance()->SendPacket(i, &p);
		}
	}

	//나에게 접속중인 다른 플레이어의 정보를 전송
	//(NPC, 플레이어 포함)
	
	for (auto& i : nearList) {
		if (g_clients[i]->is_use) {
			if (i == new_key)
				continue;
			if (!Server::getInstance()->CanSee(i, new_key))
				continue;
			p.id = i;
			p.x = g_clients[i]->x;
			p.y = g_clients[i]->y;
			newClient->vlm.lock();
			newClient->viewlist.emplace(i);
			newClient->vlm.unlock();
			//WakeUpNPC(i);
			Server::getInstance()->SendPacket(new_key, &p);
		}
	}

	SendStatPacket(new_key); //플레이어 스텟 전송
	printf("client input : %d\n", new_key);
}

void Server::SearchClientID(BYTE* id, Client* client, int index)
{
#ifdef DB
	sc_packet_login* packet = reinterpret_cast<sc_packet_login*>(id);
	
	int cID = packet->type;
	if (cID == DUMMY_CLIENT) {
		AcceptNewClient(client, index);
		return;
	}
	for (int i = 0; i < MAX_USER; ++i) {
		Client* client = reinterpret_cast<Client*>(g_clients[i]);
		if (!client->is_use) continue;
		if (client->player_id == cID) {
			sc_packet_remove_player Packet;
			Packet.id = 0;
			Packet.size = sizeof(sc_packet_remove_player);
			Packet.type = SC_DUPLICATON_PLAYER;

			SendPacket(index, &Packet);
			DisconnectPlayer(index);
			return;
		}
	}

	//DBEvent newEvent = DBEvent(SEARCH_ID, cID, client, index);
	//db_queue.push(newEvent);
	db_queue.emplace(DBEvent(SEARCH_ID, cID, client, index));
#else
	AcceptNewClient(client, index);
#endif
}

void Server::add_timer(int id, int target, int info, int type, float time)
{
	tmp.lock();
	event_queue.emplace(new Event(id, target, info, time, type, chrono::system_clock::now()));
	tmp.unlock();
}

void Server::MoveNpc(int key)
{
	Npc* thisNPC = reinterpret_cast<Npc*>(g_clients[key]);
	/*if (thisNPC->ai_work)
		return;*/

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
	/*int prev_x = g_clients[key]->zone_x;
	int prev_y = g_clients[key]->zone_y;

	g_clients[key]->zone_x = g_clients[key]->x / ZONE_INTERVAL;
	g_clients[key]->zone_y = g_clients[key]->y / ZONE_INTERVAL;

	if (g_clients[key]->zone_x != prev_x || g_clients[key]->zone_y != prev_y) {
		g_zone[prev_y][prev_x].erase(key);
		g_zone[g_clients[key]->zone_y][g_clients[key]->zone_x].insert(key);
	}*/

	sc_packet_pos posPacket;
	posPacket.id = key;
	posPacket.size = sizeof(sc_packet_pos);
	posPacket.type = SC_POS;
	posPacket.x = g_clients[key]->x;
	posPacket.y = g_clients[key]->y;


	
	thisNPC->vlm.lock();
	unordered_set<int> new_viewList = thisNPC->viewlist;
	thisNPC->vlm.unlock();

	for (auto& id : new_viewList) {
		Client* target = reinterpret_cast<Client*>(g_clients[id]);
		if (CanSee(key, id)) {
			target->vlm.lock();
			if (target->viewlist.count(key) == 0) {
				target->viewlist.emplace(key);
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

	if (!new_viewList.empty() && thisNPC->ai_work == false) {
		add_timer(key, -1, -1, MOVE_AROUND_TYPE, 1);
	}
	else {
		g_clients[key]->is_active = false;
		//thisNPC->vlm.lock();
		//thisNPC->viewlist.swap(*new unordered_set<int>());
		//thisNPC->vlm.unlock();
	}
}

void Server::MoveAINpc(int key, int target)
{
	Npc* thisNPC = reinterpret_cast<Npc*>(g_clients[key]);
	thisNPC->ai_work = true;

	if (thisNPC->state == STATE_DEATH) {
		return;
	}

	int dir_x = g_clients[target]->x - g_clients[key]->x;
	int dir_y = g_clients[target]->y - g_clients[key]->y;
	if ((dir_x*dir_x <= 1) && (dir_y*dir_y <= 1)) {
		//플레이어 공격
		/*wchar_t tmp[7] = L"attack";
		SendChatPacket(target, key, tmp);*/
		if (dir_x == 0 || dir_y == 0) {
			add_timer(key, target, -1, NPC_ATTACK_TYPE, 0.5f);
			thisNPC->state = STATE_ATTACK;
			//g_clients[key]->ai_work = false;
			return;
		}
	}
	thisNPC->state = STATE_MOVE;
	int move_x = -1, move_y = -1;
	if (dir_x >= dir_y) {
		if (dir_x != 0) {
			if (dir_x > 0)
				move_x = g_clients[key]->x + 1;
			else
				move_x = g_clients[key]->x - 1;
			if (checkCollisionMap(move_x, g_clients[key]->y))
				move_x = g_clients[key]->x;
			else
				move_y = g_clients[key]->y;
		}
		else if (dir_y != 0 && move_y == -1) {
			if (dir_y > 0)
				move_y = g_clients[key]->y + 1;
			else
				move_y = g_clients[key]->y - 1;
			if (checkCollisionMap(g_clients[key]->x, move_y))
				move_y = g_clients[key]->y;
			else
				move_x = g_clients[key]->x;
		}
	}
	else {
		if (dir_y != 0) {
			if (dir_y > 0)
				move_y = g_clients[key]->y + 1;
			else
				move_y = g_clients[key]->y - 1;
			if (checkCollisionMap(g_clients[key]->x, move_y))
				move_y = g_clients[key]->y;
			else
				move_x = g_clients[key]->x;
		}
		else if (dir_x != 0 && move_x == -1) {
			if (dir_x > 0)
				move_x = g_clients[key]->x + 1;
			else
				move_x = g_clients[key]->x - 1;
			if (checkCollisionMap(move_x, g_clients[key]->y))
				move_x = g_clients[key]->x;
			else
				move_y = g_clients[key]->y;
		}
	}
	if(move_x != -1) g_clients[key]->x = move_x;
	if(move_y != -1) g_clients[key]->y = move_y;

	sc_packet_pos posPacket;
	posPacket.id = key;
	posPacket.size = sizeof(sc_packet_pos);
	posPacket.type = SC_POS;
	posPacket.x = g_clients[key]->x;
	posPacket.y = g_clients[key]->y;

	thisNPC->vlm.lock();
	unordered_set<int> new_viewList = thisNPC->viewlist;
	thisNPC->vlm.unlock();

	for (auto& id : new_viewList) {
		Client* target = reinterpret_cast<Client*>(g_clients[id]);
		if (CanSee(key, id)) {
			target->vlm.lock();
			if (target->viewlist.count(key) == 0) {
				target->viewlist.emplace(key);
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
	//dir_x = g_clients[target]->x - g_clients[key]->x;
	//dir_y = g_clients[target]->y - g_clients[key]->y;
	//if ((dir_x*dir_x <= 1) && (dir_y*dir_y <= 1)) {
	//	//플레이어 공격
	//	/*wchar_t tmp[7] = L"attack";
	//	SendChatPacket(target, key, tmp);*/
	//	add_timer(key, target, -1, NPC_ATTACK_TYPE, 0.5f);
	//	thisNPC->state = STATE_ATTACK;
	//	//g_clients[key]->ai_work = false;
	//}

	add_timer(key, target, -1, MOVE_AI_TYPE, 1); //계속 플레이어에게 이동
	
	
	//else {
	//	thisNPC->ai_work = false;
	//	wchar_t tmp[4] = L"bye";
	//	SendChatPacket(target, key, tmp);
	//	//add_timer(key, -1, -1, MOVE_TYPE, 2);
	//}
}

void Server::MoveDirNPC(int key, int dir, int target)
{
	Npc* thisNPC = reinterpret_cast<Npc*>(g_clients[key]);
	thisNPC->ai_work = true;

	if (thisNPC->state == STATE_DEATH) {
		return;
	}
	thisNPC->state = STATE_MOVE;
	switch (dir) {
	case CS_UP:
		thisNPC->y -= 1;
		break;
	case CS_DOWN:
		thisNPC->y += 1;
		break;
	case CS_LEFT:
		thisNPC->x -= 1;
		break;
	case CS_RIGHT:
		thisNPC->x += 1;
		break;
	}

	sc_packet_pos posPacket;
	posPacket.id = key;
	posPacket.size = sizeof(sc_packet_pos);
	posPacket.type = SC_POS;
	posPacket.x = thisNPC->x;
	posPacket.y = thisNPC->y;

	thisNPC->vlm.lock();
	unordered_set<int> new_viewList = thisNPC->viewlist;
	thisNPC->vlm.unlock();

	for (auto& id : new_viewList) {
		Client* target = reinterpret_cast<Client*>(g_clients[id]);
		if (CanSee(key, id)) {
			target->vlm.lock();
			if (target->viewlist.count(key) == 0) {
				target->viewlist.emplace(key);
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
	add_timer(key, target, -1, MOVE_AI_TYPE, 1); //계속 플레이어에게 이동
}

void Server::WakeUpNPC(int id)
{
	if (!isNPC(id))
		return;
	if (g_clients[id]->is_active)
		return;
	
	g_clients[id]->is_active = true; //CAS(&is_active, false, true)
	add_timer(id, -1, -1, MOVE_AROUND_TYPE, 1.0f);
	
}

void Server::NPC_AI(int npc, int player)
{
	Npc* AI = reinterpret_cast<Npc*>(g_clients[npc]);
	if (AI->ai_work != false)
		return;
	switch (AI->type) {
	case MONSTER_PASSIVE:
		break;
	case MONSTER_ACTIVE:
		AI->aiScript->eventPlayerMove(player);
		break;
	case MONSTER_OFFENSIVE:
		WakeUpNPC(npc);
		AI->aiScript->eventPlayerMove(player);
		break;
	case MONSTER_BOSS:
		AI->aiScript->eventPlayerMove(player);
		add_timer(npc, player, 0, BOSS_SKILL, 12);
		add_timer(npc, player, 1, BOSS_SKILL, 5);
		break;
	}
}

void Server::RespawnNPC(int npc)
{
	Npc* monster = reinterpret_cast<Npc*>(g_clients[npc]);
	monster->Respawn();
	unordered_set<int> nearList = ProcessNearZone(npc);
	for (auto& player : nearList) {
		SendPutObject(player, npc);
	}
}

void Server::ReturnTown(int id)
{
	sc_packet_pos p;
	p.id = id;
	p.size = sizeof(sc_packet_pos);
	p.type = CS_GOTOWN;
	ProcessPacket(id, reinterpret_cast<char*>(&p));
}

void Server::RespwanPlayer(int id)
{
	ReturnTown(id);
	g_clients[id]->hp = g_clients[id]->level * 20;
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
	unsigned char* ptr = reinterpret_cast<unsigned char*>(exover->io_buf);

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
		/*	if (err_no != WSA_IO_PENDING)
				err_display("Recv in WorkThread", err_no);*/
		}
	}
	else {
		ProcessPacket(static_cast<int>(key), exover->io_buf);
	}
}

void Server::UploadUserDatatoDB()
{
	for (int i = 0; i < MAX_USER; ++i) {
		Client* client = reinterpret_cast<Client*>(g_clients[i]);
		if (!client->is_use) continue;

		db_queue.emplace(DBEvent(UPDATE_POS, client->player_id, client, i));
	}

	add_timer(-1, -1, -1, DB_UPDATE_TYPE, 10);
}

void Server::UpdateUserInfotoDB(int id)
{
	Client* client = reinterpret_cast<Client*>(g_clients[id]);
	//if (!client->is_use) return;

	db_queue.emplace(DBEvent(UPDATE_INFO, client->player_id, client, id));
}

bool Server::nearArea(int id, int target)
{
	int dir_x = g_clients[id]->x - g_clients[target]->x;
	int dir_y = g_clients[id]->y - g_clients[target]->y;

	if (dir_x <= 1 && dir_x >= -1)
		if (dir_y <= 1 && dir_y >= -1)
			return true;
	return false;
}

string hitMsg = "The Player hit the monster | damage : ";
string killMsg = "The Player kill the monster | exp gain : ";
void Server::PlayerAttack(int id)
{
	Client* player = reinterpret_cast<Client*>(g_clients[id]);

	if (GetTickCount() - player->coolTime[0] < 1000)
		return;
	player->coolTime[0] = GetTickCount();
	SendSkillPacket(id, 0);
	player->vlm.lock();
	unordered_set<int> nearList = player->viewlist;
	player->vlm.unlock();
	char damage = g_clients[id]->level * 3 + rand()%3;
	for (auto& npc : nearList) {
		if (g_clients[npc]->is_use == false)
			continue;

		if (nearArea(id, npc) == true) {
			g_clients[npc]->hp -= damage;
			Npc* ai = reinterpret_cast<Npc*>(g_clients[npc]);
			if (ai->state == STATE_IDLE)
				add_timer(npc, id, -1, MOVE_AI_TYPE, 0);
			string msg;
			wstring wide_string;
			if (ai->hp <= 0) {
				msg = killMsg + to_string(g_clients[npc]->level);
				ai->state = STATE_DEATH;
				ai->is_use = false;
				player->exp += g_clients[npc]->level;
				if (player->exp >= g_expTable[player->level]) {
					PlayerLevelUp(player);
				}
				add_timer(npc, -1, -1, NPC_RESPAWN_TYPE, 10);
				SendStatPacket(id);
				SendRemoveObject(id, npc);
				if (player->checkQuest(true)) {
					SendQuestClear(player, id);
				}
			}
			else {
				msg = hitMsg + to_string(damage);
			}
			wide_string.assign(msg.begin(), msg.end());
			SendChatPacket(id, id, wide_string.c_str(), INFO_ATTACK);
		}
	}
}

string coolMsg = "skill cool time is remained ";
string coolMsg2 = "sec";

void Server::PlayerSkill(int id)
{
	Client* player = reinterpret_cast<Client*>(g_clients[id]);

	DWORD tmp = GetTickCount() - player->coolTime[1];
	if (tmp < 5000) {
		string msg = coolMsg + to_string((5000 - tmp) / 1000) + coolMsg2;
		wstring wide_string;
		wide_string.assign(msg.begin(), msg.end());
		SendChatPacket(id, id, wide_string.c_str(), INFO_NONE);
		return;
	}
	player->coolTime[1] = GetTickCount();
	SendSkillPacket(id, 1);
	player->vlm.lock();
	unordered_set<int> nearList = player->viewlist;
	player->vlm.unlock();
	char damage = g_clients[id]->level * 5 + rand() % 3;
	for (auto& npc : nearList) {
		if (g_clients[npc]->is_use == false)
			continue;

		if (nearArea(id, npc) == true) {
			g_clients[npc]->hp -= damage;
			Npc* ai = reinterpret_cast<Npc*>(g_clients[npc]);
			if (ai->state == STATE_IDLE)
				add_timer(npc, id, -1, MOVE_AI_TYPE, 0);
			string msg;
			wstring wide_string;
			if (ai->hp <= 0) {
				msg = killMsg + to_string(g_clients[npc]->level);
				ai->state = STATE_DEATH;
				ai->is_use = false;
				player->exp += g_clients[npc]->level;
				if (player->exp >= g_expTable[player->level]) {
					PlayerLevelUp(player);
				}
				add_timer(npc, -1, -1, NPC_RESPAWN_TYPE, 10);
				SendStatPacket(id);
				SendRemoveObject(id, npc);
			}
			else {
				msg = hitMsg + to_string(damage);
			}
			wide_string.assign(msg.begin(), msg.end());
			SendChatPacket(id, id, wide_string.c_str(), INFO_ATTACK);

			int dir_x = player->x - g_clients[npc]->x;
			int dir_y = player->y - g_clients[npc]->y;
			if (dir_x>= 1) 
				add_timer(npc, id, CS_LEFT, MOVE_DIR_TYPE, 0);
			else if(dir_x<=-1)
				add_timer(npc, id, CS_RIGHT, MOVE_DIR_TYPE, 0);
			else if(dir_y>=1)
				add_timer(npc, id, CS_UP, MOVE_DIR_TYPE, 0);
			else if(dir_y<=-1)
				add_timer(npc, id, CS_DOWN, MOVE_DIR_TYPE, 0);
		}
	}
}
void Server::PlayerSkill2(int id)
{
	Client* player = reinterpret_cast<Client*>(g_clients[id]);

	DWORD tmp = GetTickCount() - player->coolTime[2];
	if (tmp < 5000) {
		string msg = coolMsg + to_string((5000 - tmp) / 1000) + coolMsg2;
		wstring wide_string;
		wide_string.assign(msg.begin(), msg.end());
		SendChatPacket(id, id, wide_string.c_str(), INFO_NONE);
		return;
	}
	player->coolTime[2] = GetTickCount();
	SendSkillPacket(id, 2);
	g_clients[id]->hp += g_clients[id]->level * 2;
	if (g_clients[id]->hp > g_clients[id]->level * 20)
		g_clients[id]->hp = g_clients[id]->level * 20;
	SendStatPacket(id);
}
string levelmsg = "Player Level UP!";
void Server::PlayerLevelUp(Client* player)
{
	player->level += 1;
	player->exp = 0;
	player->hp = player->level * 20;
	wstring wide_string;

	wide_string.assign(levelmsg.begin(), levelmsg.end());
	SendChatPacket(player->player_id, player->player_id, wide_string.c_str(), INFO_NONE);
}

string hitMsg2 = "The Monster hit player | damage : ";
string killMsg2 = "The Monster kill player... return to town";

void Server::NPCAttack(int id, int target)
{
	Npc* npc = reinterpret_cast<Npc*>(g_clients[id]);
	if (npc->state == STATE_DEATH)
		return;
	npc->state = STATE_ATTACK;
	npc->ai_work = true;
	npc->vlm.lock();
	unordered_set<int> nearList = npc->viewlist;
	npc->vlm.unlock();
	char damage = npc->level * 3 + rand() % 3;
	if (nearArea(id, target) == true) {
		g_clients[target]->hp -= damage;
		string msg;
		wstring wide_string;
		if (g_clients[target]->hp < 0) {
			msg = killMsg2;
			npc->ai_work = false;
			npc->state = STATE_IDLE;
			RespwanPlayer(target);
		}
		else {
			msg = hitMsg2 + to_string(damage);
			add_timer(id, target, 01, NPC_ATTACK_TYPE, 0.5f);
		}
		wchar_t tmp[7] = L"attack";
		SendChatPacket(target, id, tmp, INFO_ATTACK);
		wide_string.assign(msg.begin(), msg.end());
		SendChatPacket(target, target, wide_string.c_str(), INFO_NONE);
		SendStatPacket(target);
	}
	else {
		add_timer(id, target, -1, MOVE_AI_TYPE, 1); //계속 플레이어에게 이동
	}
}

int targetPoint_x[4] = { 0,0,3,-3 };
int targetPoint_y[4] = { 3,-3,0,0 };

void Server::BossSkill(int id, int target)
{
	Npc* boss = reinterpret_cast<Npc*>(g_clients[id]);
	if (boss->is_use == false)
		return;
	if (boss->state == STATE_IDLE)
		return;
	boss->state = STATE_ATTACK;
	wchar_t tmp[6] = L"slash";
	SendChatPacket(target, id, tmp, INFO_ATTACK);
	sc_packet_boss p;
	p.type = SC_BOSS_SKILL;
	p.size = sizeof(p);
	p.kind = 0;
	for (int i = 0; i < 4; ++i) {
		p.x = boss->x + targetPoint_x[i];
		p.y = boss->y + targetPoint_y[i];
		SendPacket(target, &p);
	}
	int dir_x = g_clients[target]->x - g_clients[id]->x;
	int dir_y = g_clients[target]->y - g_clients[id]->y;
	if (dir_x*dir_x + dir_y*dir_y < 18) {
		g_clients[target]->hp -= 400;
		string msg;
		wstring wide_string;
		if (g_clients[target]->hp < 0) {
			msg = killMsg2;
			boss->ai_work = false;
			boss->state = STATE_IDLE;
			RespwanPlayer(target);
		}
		else {
			msg = hitMsg2 + to_string(400);
		}

		wide_string.assign(msg.begin(), msg.end());
		SendChatPacket(target, target, wide_string.c_str(), INFO_NONE);
		SendStatPacket(target);
	}
	add_timer(id, target, 0, BOSS_SKILL, 12);
}

void Server::BossSkill2(int id, int target)
{
	Npc* boss = reinterpret_cast<Npc*>(g_clients[id]);
	if (boss->is_use == false)
		return;
	if (boss->state == STATE_IDLE)
		return;
	wchar_t tmp[9] = L"darkball";
	SendChatPacket(target, id, tmp, INFO_ATTACK);
	//wide_string.assign(msg.begin(), msg.end());
	//SendChatPacket(target, target, wide_string.c_str(), INFO_NONE);
	

	sc_packet_boss p;
	p.type = SC_BOSS_SKILL;
	p.size = sizeof(p);
	p.kind = 1;
	p.x = boss->x;
	p.y = boss->y;
	SendPacket(target, &p);

	if (nearArea(id, target) == true) {
		g_clients[target]->hp -= 100;

		Client* player = reinterpret_cast<Client*>(g_clients[target]);
		player->stun = true;
		player->stunTime = GetTickCount();
		SendChatPacket(target, target, L"Crowd Control : Stun", INFO_NONE);

		string msg;
		wstring wide_string;
		if (g_clients[target]->hp < 0) {
			msg = killMsg2;
			boss->ai_work = false;
			boss->state = STATE_IDLE;
			RespwanPlayer(target);
		}
		else {
			msg = hitMsg2 + to_string(100);
		}
	
		wide_string.assign(msg.begin(), msg.end());
		SendChatPacket(target, target, wide_string.c_str(), INFO_NONE);
		SendStatPacket(target);
	}

	add_timer(id, target, 1, BOSS_SKILL, 5);
}

bool Server::CheckPlayerLevel(Client* player)
{
	int xPos = player->x;
	int yPos = player->y;
	int level = player->level;

	if (xPos > 127 && xPos < 131 && yPos == 95) {
		if (level < 11)
			return false;
	}
	else if (xPos > 125 && xPos < 129 && yPos == 234) {
		if (level < 30)
			return false;
	}
	else if (xPos == 216 && yPos > 189 && yPos < 193) {
		if (level < 21)
			return false;
	}
	return true;
}
WCHAR quest_hunt1[31] = L"hunt monster type 0 | remain 1";
WCHAR quest_hunt2[31] = L"hunt monster type 1 | remain 1";
WCHAR quest_hunt3[31] = L"hunt monster type 2 | remain 1";
WCHAR quest_hunt4[29] = L"hunt monster boss | remain 1";
WCHAR quest_level1[16] = L"goal level | 10";
WCHAR quest_level2[16] = L"goal level | 20";
//WCHAR quest_clear[7] = L"Clear!";

WCHAR* questStr[6] = { quest_hunt1, quest_hunt2, quest_hunt3, quest_hunt4, quest_level1, quest_level2 };

int quest_x[6] = { 102, 107, 112, 117, 112, 117 };
int quest_y[6] = { 185, 185, 185, 185, 198, 198 };

void Server::CheckQuest(Client * player, int id)
{
	for (int i = 0; i < 6; ++i) {
		if (player->x == quest_x[i] && player->y == quest_y[i]) {
			if (player->setQuest(&quest[i], i) == false) {
				SendChatPacket(id, id, L"Quest already exist", INFO_NONE);
				return;
			}
			SendQuestPacket(player, id);
			if (player->checkQuest(false))
				SendQuestClear(player, id);
			return;
		}
	}
}

void Server::SendQuestPacket(Client * player, int id)
{
	if (player->myQuest == nullptr)
		return;

	SendChatPacket(id, id, questStr[player->myQuest->type], INFO_QUEST);
}

void Server::SendQuestClear(Client * player, int id)
{
	SendChatPacket(id, id, L"Quest Clear! | exp + 10", INFO_CLEAR);
	player->exp += 10;
	SendStatPacket(id);
}

int CAPI_getX(lua_State * L)
{
	int player = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int x = Server::getInstance()->g_clients[player]->x;
	lua_pushnumber(L, x);

	return 1;
}

int CAPI_getY(lua_State * L)
{
	int player = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int y = Server::getInstance()->g_clients[player]->y;
	lua_pushnumber(L, y);

	return 1;
}

int CAPI_sendMsg(lua_State * L)
{
	char* msg = (char*)lua_tostring(L, -1);
	int chatter = (int)lua_tointeger(L, -2);
	int player = (int)lua_tointeger(L, -3);
	lua_pop(L, 4);

	wchar_t buf[MAX_STR_SIZE];
	size_t len = strlen(msg);
	if (len >= MAX_STR_SIZE)
		len = MAX_STR_SIZE - 1;

	size_t wlen;
	mbstowcs_s(&wlen, buf, len, msg, _TRUNCATE);
	buf[MAX_STR_SIZE - 1] = (wchar_t)0;

	Server::getInstance()->SendChatPacket(player, chatter, buf, INFO_NONE);
	return 0;
}

int CAPI_moveNPC(lua_State * L)
{
	int npc = (int)lua_tointeger(L, -1);
	int player = (int)lua_tointeger(L, -2);
	Server::getInstance()->g_clients[npc]->ai_work = true;
	lua_pop(L, 3);
	Server::getInstance()->add_timer(npc, player, -1, MOVE_AI_TYPE, 1);
	return 0;
}
