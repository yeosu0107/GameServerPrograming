#pragma once
#include "stdafx.h"
#include "Objects.h"

class GlobalVal {
private:
	GlobalVal() {}
	static GlobalVal*				g_instance;

	Objects*						m_player;
	Objects**						m_other;
	Objects**						m_npc;

	int*								g_xPos;
	int*								g_yPos;
public:
	~GlobalVal();
	static GlobalVal* getInstance();

	void SetObject(Objects* player, Objects** other, Objects** npc);
	void SetGlobalPos(int* x, int* y);

	Objects* getPlayer() { return m_player; }
	Objects** getOther() { return m_other; }
	Objects** getNpc() { return m_npc; }

	int* getxPos() { return g_xPos; }
	int* getyPos() { return g_yPos; }
};