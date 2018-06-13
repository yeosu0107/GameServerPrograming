#include "Effect.h"
#define TILE_WIDTH 32
Effect::Effect(int id, int width, int height, int num)
{
	texture_id = id;
	nFrame = num;
	m_width = width;
	m_height = height;
}

void Effect::update(int x, int y)
{
	int xPos = (x - g_left_x) * TILE_WIDTH;
	int yPos = (y - g_top_y) * TILE_WIDTH;

	pos1 = D3DXVECTOR3(xPos - TILE_WIDTH, yPos, 0);
	pos2 = D3DXVECTOR3(xPos + TILE_WIDTH, yPos, 0);
	pos3 = D3DXVECTOR3(xPos, yPos - TILE_WIDTH, 0);
	pos4 = D3DXVECTOR3(xPos, yPos + TILE_WIDTH, 0);
	now_render = true;
	now_frame = 0;
}

void Effect::draw()
{
	now_frame += 1;
	if (now_frame > nFrame - 1) {
		now_frame = 0;
		now_render = false;
	}
	drawRect = { now_frame * m_width, 0,
		(now_frame + 1) * m_width, m_height };

	g_pSprite->Draw(g_textures[texture_id], &drawRect, NULL, &pos1 ,
		D3DCOLOR_ARGB(255, 255, 255, 255));
	g_pSprite->Draw(g_textures[texture_id], &drawRect, NULL, &pos2,
		D3DCOLOR_ARGB(255, 255, 255, 255));
	g_pSprite->Draw(g_textures[texture_id], &drawRect, NULL, &pos3,
		D3DCOLOR_ARGB(255, 255, 255, 255));
	g_pSprite->Draw(g_textures[texture_id], &drawRect, NULL, &pos4,
		D3DCOLOR_ARGB(255, 255, 255, 255));
}
