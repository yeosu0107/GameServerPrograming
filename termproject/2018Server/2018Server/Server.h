#pragma once
#include <vector>
#include <queue>
#include "protocol.h"
#include "Client.h"
#include "DBConnect.h"
#include "CollisionMap.h"
using namespace std;

const int NONE_TYPE = 0;
const int MOVE_AROUND_TYPE = 1;
const int MOVE_AI_TYPE = 2;
const int DB_UPDATE_TYPE = 3;
const int NPC_RESPAWN_TYPE = 4;
const int NPC_ATTACK_TYPE = 5;
const int MOVE_DIR_TYPE = 6;



struct Event {
	int id;
	float time;
	int type;
	chrono::system_clock::time_point startClock;

	int target;
	int info;
	
	Event(int _id, int _target, int _info, float _time, int _type, chrono::system_clock::time_point _Clock) :
		id(_id), target(_target), info(_info), time(_time), type(_type), startClock(_Clock)
	{}
};

struct DBEvent {
	int type;
	int id;
	Client* client;
	int clientIndex;

	DBEvent(int type, int id, Client* cl, int index) {
		this->type = type;
		this->id = id;
		this->client = cl;
		this->clientIndex = index;
	}
};

struct compare {
private:
	bool reserve;
public:
	compare() { }
	bool operator()(const Event* t1, const Event* t2) const{
		return t1->startClock > t2->startClock;
	}
};

class Server
{
private:
	Server() {}

	static Server* g_server;

	HANDLE		g_iocp;

	priority_queue<Event*, vector<Event*>, compare> event_queue;
	queue<DBEvent> db_queue;
	mutex tmp;

	
	unordered_set<int> g_zone[11][11];
	mutex g_mutex[11][11];

	int (*g_collisionMap)[300];
	queue<spawnPoint> g_spawnPoint;
	vector<int> g_expTable;
	int g_MaxLevel;

	bool isNPC(int index);
public:
	vector<Object*> g_clients;

	~Server();
	static Server* getInstance();

	void Initialize();

	bool CanSee(int cl1, int cl2);
	void addViewList(unordered_set<int>& viewList, const int clientID, const int x, const int y);
	unordered_set<int> ProcessNearZone(int key);
	bool checkCollisionMap(int index);
	bool checkCollisionMap(int x, int y);

	void SendPacket(int id, void* packet);
	void SendPutObject(int client, int object);
	void SendRemoveObject(int client, int object);
	void SendChatPacket(int to, int from, const wchar_t* msg, int info);
	void SendStatPacket(int id);
	void DisconnectPlayer(int id);
	void ProcessPacket(int clientID, char* packet);

	void AcceptAndSearchClient(SOCKET& g_socket);
	void AcceptNewClient(Client* client, int key);
	void SearchClientID(BYTE* id, Client* client, int index);

	void add_timer(int id, int target, int info, int type, float time);
	void MoveNpc(int id);
	void MoveAINpc(int id, int target);
	void MoveDirNPC(int id, int dir, int target);
	void WakeUpNPC(int id);
	void NPC_AI(int npc, int player);
	void RespawnNPC(int npc);

	void ReturnTown(int id);
	void RespwanPlayer(int id);

	Object* getClient(int id);
	HANDLE* getIOCP();
	priority_queue<Event*, vector<Event*>, compare>* getEventQueue(){ return &event_queue; }
	queue<DBEvent>* getDB() { return&db_queue; }
	mutex* getMutex() { return &tmp; }
	void recv(unsigned long long& key, unsigned long& data_size, EXOver* exover);

	void UploadUserDatatoDB();

	bool nearArea(int id, int target);
	void PlayerAttack(int id);
	void PlayerSkill(int id);
	void PlayerLevelUp(Client* player);
	void NPCAttack(int id, int target);
};

int CAPI_getX(lua_State* L);
int CAPI_getY(lua_State* L);
int CAPI_sendMsg(lua_State* L);
int CAPI_moveNPC(lua_State* L);