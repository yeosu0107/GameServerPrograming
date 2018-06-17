#pragma once
#define MAX_BUFF_SIZE   4000
#define MAX_PACKET_SIZE  255

#define BOARD_WIDTH   300
#define BOARD_HEIGHT  300

#define VIEW_RADIUS   11

#define MAX_USER 1000		//ID (0~9)

#define NPC_START  1000		//NPC ID
#define NUM_OF_NPC  1000 + 1597		//유저 + NPC의 수

#define MY_SERVER_PORT  4000

#define MAX_STR_SIZE  100

#define CS_UP    1
#define CS_DOWN  2
#define CS_LEFT  3
#define CS_RIGHT    4
#define CS_CHAT		5
#define CS_RANDOM 6
#define CS_ATTACK 8
#define CS_GOTOWN 7
#define CS_SKILL1 9
#define CS_SKILL2 10

#define SC_POS           1
#define SC_PUT_PLAYER    2
#define SC_REMOVE_PLAYER 3
#define SC_CHAT			4
#define SC_DUPLICATON_PLAYER 5
#define SC_ATTACK 6
#define SC_PLAYER_STAT 7
#define SC_PLAYER_SKILL 8
#define SC_BOSS_SKILL 9

#define ZONE_INTERVAL 30
#define ZONE_EDGH  23

#define INFO_NONE 0
#define INFO_ATTACK 1
#define INFO_SKILL1 2
#define INFO_SKILL2 3

#pragma pack (push, 1)

struct cs_packet_up {
	BYTE size;
	BYTE type;
};

struct cs_packet_down {
	BYTE size;
	BYTE type;
};

struct cs_packet_left {
	BYTE size;
	BYTE type;
};

struct cs_packet_right {
	BYTE size;
	BYTE type;
};

struct cs_packet_chat {
	BYTE size;
	BYTE type;
	WCHAR message[MAX_STR_SIZE];
};

struct sc_packet_pos {
	BYTE size;
	BYTE type;
	WORD id;
	UINT x;
	UINT y;
};

struct sc_packet_put_player {
	BYTE size;
	BYTE type;
	WORD id;
	WORD pic_type;
	UINT x;
	UINT y;
};
struct sc_packet_remove_player {
	BYTE size;
	BYTE type;
	WORD id;
};

struct sc_packet_chat {
	BYTE size;
	BYTE type;
	WORD id;
	WORD info;
	WCHAR message[MAX_STR_SIZE];
};

struct sc_packet_login {
	BYTE size;
	BYTE type;
	WCHAR char_str[100];
};

struct sc_packet_stat {
	BYTE size;
	BYTE type;
	WORD hp;
	WORD level;
	DWORD exp;
	DWORD max_exp;
};

struct sc_packet_skill {
	BYTE size;
	BYTE type;
	DWORD kind;
};

struct sc_packet_boss {
	BYTE size;
	BYTE type;
	DWORD kind;
	UINT x;
	UINT y;
};
#pragma pack (pop)