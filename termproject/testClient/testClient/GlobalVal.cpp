#include "stdafx.h"
#include "GlobalVal.h"


GlobalVal* GlobalVal::g_instance = nullptr;

GlobalVal::~GlobalVal()
{
}

GlobalVal * GlobalVal::getInstance()
{
	if (!g_instance)
		g_instance = new GlobalVal();
	return g_instance;
}

void GlobalVal::SetObject(Objects* player, Objects** other, Objects** npc)
{
	m_player = player;
	m_other = other;
	m_npc = npc;
}

void GlobalVal::SetGlobalPos(int * x, int * y)
{
	g_xPos = x;
	g_yPos = y;
}
