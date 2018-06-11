#pragma once
#include <mutex>
#include <unordered_set>

#include "protocol.h"
#include "ScriptEngine.h"

using namespace std;

class Object {
public:
	bool is_use = false;
	bool is_active = false;
	bool ai_work = false;
	int type = 0;
	int x, y;

	int zone_x, zone_y;
};

class Npc : public Object {
public:
	bool is_live;
	mutex vlm;
	unordered_set<int> viewlist;
	int hp, mp;
	ScriptEngine* aiScript;
	
	Npc() {}

	Npc(int xPos, int yPos, ScriptEngine* ai)
	{
		x = xPos;
		y = yPos;
		is_use = true;
		is_active = false;
		zone_x = x / ZONE_INTERVAL;
		zone_y = y / ZONE_INTERVAL;

		aiScript = ai;
		ai_work = false;
	}
};

static const char EV_RECV = 0;
static const char EV_SEND = 1;
static const char EV_MOVE = 2;
static const char EV_DBUPDATE = 3;
static const char EV_PLAYER_MOVE = 4;
static const char EV_MOVE_DIR = 5;

struct EXOver {
	WSAOVERLAPPED wsaover;
	char event_type;
	int event_target;
	int event_info;
	int event_info2;
	WSABUF wsabuf;
	char io_buf[MAX_BUFF_SIZE];
};

class Client : public Npc {
public:
	//unordered_set<int> viewlist;
	//mutex vlm;
	
	int player_id;
	//for io func
	SOCKET s;
	EXOver exover;
	int packet_size;
	int prev_size;
	char prev_packet[MAX_PACKET_SIZE];

	Client() {
		exover.event_type = EV_RECV;
		exover.wsabuf.buf = exover.io_buf;
		exover.wsabuf.len = sizeof(exover.io_buf);
		packet_size = 0;
		prev_size = 0;

		aiScript = nullptr;
		ai_work = false;
	}
	Client(const Client& my) {
		exover.event_type = EV_RECV;
		exover.wsabuf.buf = exover.io_buf;
		exover.wsabuf.len = sizeof(exover.io_buf);
		packet_size = 0;
		prev_size = 0;

		aiScript = nullptr;
		ai_work = false;
	}
};

