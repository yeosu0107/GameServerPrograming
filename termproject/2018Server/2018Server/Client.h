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
	int level = 5;
	int hp = 100;
	int zone_x, zone_y;
};

const char MONSTER_PASSIVE = 0;
const char MONSTER_ACTIVE = 1;
const char MONSTER_OFFENSIVE = 2;
const char MONSTER_BOSS = 3;

const char STATE_IDLE = 0;
const char STATE_MOVE = 1;
const char STATE_ATTACK = 2;
const char STATE_DEATH = 3;

class Npc : public Object {
public:
	bool is_live;
	mutex vlm;
	unordered_set<int> viewlist;

	char type;
	char state;
	
	ScriptEngine* aiScript;
	
	int start_x, start_y;

	Npc() {}

	Npc(int xPos, int yPos, ScriptEngine* ai, char _t)
	{
		x = xPos;
		y = yPos;
		is_use = true;
		is_active = false;
		zone_x = x / ZONE_INTERVAL;
		zone_y = y / ZONE_INTERVAL;
		start_x = xPos;
		start_y = yPos;

		aiScript = ai;
		ai_work = false;
		type = _t;
		
		switch (type) {
		case MONSTER_PASSIVE:
			level = rand() % 10;
			break;
		case MONSTER_ACTIVE:
			level = rand() % 10 + 10;
			break;
		case MONSTER_OFFENSIVE:
			level = rand() % 10 + 20;
			break;
		case MONSTER_BOSS:
			level = 30;
			break;
		}
		hp = level * 9;

		state = STATE_IDLE;
	}

	void Respawn();
};

static const char EV_RECV = 0;
static const char EV_SEND = 1;
static const char EV_MOVE = 2;
static const char EV_DBUPDATE = 3;
static const char EV_PLAYER_MOVE = 4;
static const char EV_MOVE_AI = 5;
static const char EV_NPC_RESPAWN = 6;
static const char EV_NPC_ATTACK = 7;
static const char EV_MOVE_DIR = 8;

struct EXOver {
	WSAOVERLAPPED wsaover;
	char event_type;
	int event_target;
	int event_info;
	WSABUF wsabuf;
	char io_buf[MAX_BUFF_SIZE];
};

class Client : public Npc {
public:
	int player_id;
	
	int exp;
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

