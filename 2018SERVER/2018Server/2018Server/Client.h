#pragma once
#include <mutex>
#include <unordered_set>

#include "protocol.h"

using namespace std;

class Object {
public:
	SOCKET s;
	bool is_use = false;
	int type = 0;
	int x, y;

	int zone_x, zone_y;
};

struct EXOver {
	WSAOVERLAPPED wsaover;
	bool is_recv;
	WSABUF wsabuf;
	char io_buf[MAX_BUFF_SIZE];
};

class Client : public Object {
public:
	unordered_set<int> viewlist;
	mutex vlm;

	//for io func
	EXOver exover;
	int packet_size;
	int prev_size;
	char prev_packet[MAX_PACKET_SIZE];

	Client() {
		exover.is_recv = true;
		exover.wsabuf.buf = exover.io_buf;
		exover.wsabuf.len = sizeof(exover.io_buf);
		packet_size = 0;
		prev_size = 0;
	}
	Client(const Client& my) {
		exover.is_recv = true;
		exover.wsabuf.buf = exover.io_buf;
		exover.wsabuf.len = sizeof(exover.io_buf);
		packet_size = 0;
		prev_size = 0;
	}
};