#include "monster.h"

void Monster::Initialize(int x, int y)
{
	object.x = x;
	object.y = y;
	object.attr |= BOB_ATTR_VISIBLE;
}

void Monster::setPos(int x, int y)
{
	object.x = x;
	object.y = y;
}

void Monster::setDir(int x, int y)
{
	attack_dir = 0;
	if (object.x > x)
		attack_dir = attack_left;
	else if (object.x < x)
		attack_dir = attack_right;
	else if (object.y > y)
		attack_dir = attack_up;
	else if (object.y < y)
		attack_dir = attack_down;
}

void Monster::attack()
{
	if (attack_dir == 0)
		return;
	switch (attack_dir) {
	case attack_up:
		attack_y -= dir;
		break;
	case attack_down:
		attack_y += dir;
		break;
	case attack_right:
		attack_x += dir;
		break;
	case attack_left:
		attack_x -= dir;
		break;
	}
	if (attack_x * attack_x >= 64 || attack_y * attack_y >= 64)
		dir *= -1;
	if (attack_x == 0 && attack_y == 0) {
		attack_dir = 0;
		dir = 1;
	}
}

void Monster::draw()
{
	if (!&object)
		return;

	// is bob visible
	if (!(object.attr & BOB_ATTR_VISIBLE))
		return;

	attack();
	Animate_BOB32(&object);

	D3DXVECTOR3 pos = D3DXVECTOR3((object.x - g_left_x) * 32.0f + 8 + attack_x,
		(object.y - g_top_y) * 32.0f + 8 + attack_y, 0.0);

	RECT my = { object.tx[object.curr_frame], object.ty[object.curr_frame],
		object.tx[object.curr_frame] + object.width, object.ty[object.curr_frame] + object.height };

	g_pSprite->Draw(object.images[object.curr_frame], &my, NULL, &pos,
		D3DCOLOR_ARGB(255, 255, 255, 255));

	if (object.message_time > GetTickCount() - 2000)
		Draw_Text_D3D(object.message, static_cast<int>(pos.x), static_cast<int>(pos.y), D3DCOLOR_ARGB(255, 200, 200, 255));
}
