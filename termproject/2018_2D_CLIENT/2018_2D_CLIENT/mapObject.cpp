#include "mapObject.h"

#define TILE_WIDTH 32

MapObject::MapObject(int id)
{
	texture_id = id;
}

void MapObject::setRect(int x, int y)
{
	drawRect = { x * TILE_WIDTH, y * TILE_WIDTH, 
		x*TILE_WIDTH + mapTile* TILE_WIDTH, y*TILE_WIDTH + mapTile* TILE_WIDTH };
}

void MapObject::draw()
{
	g_pSprite->Draw(g_textures[texture_id], &drawRect, NULL, &pos,
		D3DCOLOR_ARGB(255, 255, 255, 255));
}
