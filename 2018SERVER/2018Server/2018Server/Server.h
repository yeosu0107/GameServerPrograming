#pragma once
#include <vector>
#include "protocol.h"
#include "Client.h"

using namespace std;

class Server
{
private:
	Server() {}

	static Server* g_server;

	HANDLE		g_iocp;
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

	Client* getClient(int id);
	HANDLE* getIOCP();
	
	void recv(unsigned long long& key, unsigned long& data_size, EXOver* exover);
};