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

bool Client::setQuest(Quest * quest, int index)
{
	if (myQuest != nullptr)
		return false;
	myQuest = quest;
	if (index < 4)
		count = 0;
	else
		count = level;
	return true;
}

bool Client::checkQuest(bool kill)
{
	if (myQuest != nullptr) {
		if (myQuest->type < 4) {
			if(kill == true)
				count += 1;
		}
		else {
			count = level;		
		}

		if (count >= myQuest->num) {
			myQuest = nullptr;
			count = 0;
			return true;
		}
	}

	return false;
}
