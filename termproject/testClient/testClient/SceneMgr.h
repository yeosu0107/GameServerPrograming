#pragma once
#include "Objects.h"
#include "Sound.h"
#include <list>

const int MAX_RENDER_TILE = 20;

const int MAX_OBJECT_COUNT = 1500;
const int MAX_TEXTURE_COUNT = 9;
const int MAX_SOUND_COUNT = 10;

class SceneMgr
{
private:
	enum ObjectType { tile0, tile1, player, other, npc};

	Renderer*				m_Renderer;
	Sound*					m_Sound;

	UINT						m_texImage[MAX_TEXTURE_COUNT];
	int							m_soundIndex[MAX_SOUND_COUNT];

	Objects***				m_mapTile;

	Objects*				m_player = nullptr;
	Objects*				m_other[MAX_USER];
	Objects*				m_npc[NUM_OF_NPC];

	int							g_xPos = 0;
	int							g_yPos = 0;
public:
	SceneMgr();
	~SceneMgr();
	
	void BuildObjects();
	void Render();
	void Update(float ElapsedTime);
	
	void MouseInput(int x, int y);

	Objects* AddObject(float3 pos, float3 dir, int type, int texIndex);
};