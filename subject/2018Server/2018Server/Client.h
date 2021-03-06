#pragma once
#include <mutex>
#include <unordered_set>

#include "protocol.h"

using namespace std;

class Object {
public:
	bool is_use = false;
	bool is_active = false;
	int type = 0;
	int x, y;

	int zone_x, zone_y;
};

class Npc : public Object {
public:
	bool is_live;
	

	int hp, mp;
};

static const char EV_RECV = 0;
static const char EV_SEND = 1;
static const char EV_MOVE = 2;

struct EXOver {
	WSAOVERLAPPED wsaover;
	char event_type;
	WSABUF wsabuf;
	char io_buf[MAX_BUFF_SIZE];
};

class Client : public Npc {
public:
	unordered_set<int> viewlist;
	mutex vlm;
	
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
	}
	Client(const Client& my) {
		exover.event_type = EV_RECV;
		exover.wsabuf.buf = exover.io_buf;
		exover.wsabuf.len = sizeof(exover.io_buf);
		packet_size = 0;
		prev_size = 0;
	}
};

