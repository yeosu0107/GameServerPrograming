#pragma once

#include <windows.h>   // include important windows stuff
#include <windowsx.h> 
#include <stdio.h>

#include <d3d9.h>  // directX includes
#include <d3dx9tex.h>  // directX includes

#include "gpdumb1.h"

const static int attack_up = 1;
const static int attack_down = 2;
const static int attack_right = 3;
const static int attack_left = 4;


struct Monster
{
	BOB object;

	int attack_x = 0, attack_y = 0;
	int attack_dir = 0;
	int dir = 1;

	void Initialize(int x, int y);

	void setPos(int x, int y);

	void setDir(int x, int y);
	void attack();
	
	void draw();
};