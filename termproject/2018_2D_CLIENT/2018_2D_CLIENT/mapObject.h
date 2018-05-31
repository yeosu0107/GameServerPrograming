#pragma once

#include <windows.h>   // include important windows stuff
#include <windowsx.h> 
#include <stdio.h>

#include <d3d9.h>  // directX includes
#include <d3dx9tex.h>  // directX includes

#include "gpdumb1.h"

const int mapTile = 21;

struct MapObject
{
	RECT drawRect;
	int texture_id;
	D3DXVECTOR3 pos = D3DXVECTOR3(0, 0, 0);

	MapObject(int id);
	void setRect(int x, int y);
	void draw();
};