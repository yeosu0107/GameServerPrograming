#pragma once

#include <windows.h>   // include important windows stuff
#include <windowsx.h> 
#include <stdio.h>

#include <d3d9.h>  // directX includes
#include <d3dx9tex.h>  // directX includes

#include "gpdumb1.h"

struct Effect
{
	RECT drawRect;
	int texture_id;
	D3DXVECTOR3 pos1 = D3DXVECTOR3(0, 0, 0);
	D3DXVECTOR3 pos2 = D3DXVECTOR3(0, 0, 0);
	D3DXVECTOR3 pos3 = D3DXVECTOR3(0, 0, 0);
	D3DXVECTOR3 pos4 = D3DXVECTOR3(0, 0, 0);
	unsigned char nFrame;
	unsigned char now_frame;
	int m_width, m_height;

	bool now_render = false;

	Effect(int id, int width, int height, int num);

	void update(int x, int y);
	void draw();
};