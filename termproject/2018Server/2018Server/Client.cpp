#include "stdafx.h"
#include "Client.h"

void Npc::Respawn()
{
	x = start_x;
	y = start_y;
	is_use = true;
	is_active = false;
	ai_work = false;
	zone_x = x / ZONE_INTERVAL;
	zone_y = y / ZONE_INTERVAL;

	hp = level * 9;

	state = STATE_IDLE;
}
