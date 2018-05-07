#pragma once
#include <vector>
#include <queue>
#include "protocol.h"
#include "Client.h"

using namespace std;

const int NONE_TYPE = 0;
const int MOVE_TYPE = 1;
const int RESERVE_TYPE = 2;

struct Event {
	int id;
	float time;
	int type;
	chrono::system_clock::time_point startClock;
};

struct compare {
	bool operator()(Event t1, Event t2) {
		return t1.startClock > t2.startClock;
	}
};

class Server
{
private:
	Server() {}

	static Server* g_server;

	HANDLE		g_iocp;

	priority_queue<Event, vector<Event>, compare> event_queue;
	mutex tmp;

	vector<Client> g_clients;
	unordered_set<int> g_zone[ZONE_INTERVAL][ZONE_INTERVAL];
public:
	~Server();
	static Server* getInstance();

	void Initialize();

	bool CanSee(int cl1, int cl2);
	void addViewList(unordered_set<int>& viewList, const int clientID, const int x, const int y);

	void SendPacket(int id, void* packet);
	void SendPutObject(int client, int object);
	void SendRemoveObject(int client, int object);
	void DisconnectPlayer(int id);
	void ProcessPacket(int clientID, char* packet);

	void AcceptNewClient(SOCKET& g_socket);

	void add_timer(int id, int type, float time);
	void MoveNpc();

	Client* getClient(int id);
	HANDLE* getIOCP();
	priority_queue<Event, vector<Event>, compare>* getEventQueue(){ return &event_queue; }
	mutex* getMutex() { return &tmp; }
	void recv(unsigned long long& key, unsigned long& data_size, EXOver* exover);
};