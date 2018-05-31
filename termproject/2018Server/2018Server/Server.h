#pragma once
#include <vector>
#include <queue>
#include "protocol.h"
#include "Client.h"
#include "DBConnect.h"

using namespace std;

const int NONE_TYPE = 0;
const int MOVE_TYPE = 1;
const int DB_UPDATE_TYPE = 2;

struct Event {
	int id;
	float time;
	int type;
	chrono::system_clock::time_point startClock;

	int target;
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

	vector<Object*> g_clients;
	unordered_set<int> g_zone[ZONE_INTERVAL][ZONE_INTERVAL];

	int(*g_collisionMap)[300];

	bool isNPC(int index);
public:
	~Server();
	static Server* getInstance();

	void Initialize();

	bool CanSee(int cl1, int cl2);
	void addViewList(unordered_set<int>& viewList, const int clientID, const int x, const int y);
	unordered_set<int> ProcessNearZone(int key);

	void SendPacket(int id, void* packet);
	void SendPutObject(int client, int object);
	void SendRemoveObject(int client, int object);
	void DisconnectPlayer(int id);
	void ProcessPacket(int clientID, char* packet);

	void AcceptAndSearchClient(SOCKET& g_socket);
	void AcceptNewClient(Client* client, int key);
	void SearchClientID(BYTE* id, Client* client, int index);

	void add_timer(int id, int type, float time);
	void MoveNpc(int id);
	void WakeUpNPC(int id);

	Object* getClient(int id);
	HANDLE* getIOCP();
	priority_queue<Event*, vector<Event*>, compare>* getEventQueue(){ return &event_queue; }
	queue<DBEvent>* getDB() { return&db_queue; }
	mutex* getMutex() { return &tmp; }
	void recv(unsigned long long& key, unsigned long& data_size, EXOver* exover);

	void UploadUserDatatoDB();
};