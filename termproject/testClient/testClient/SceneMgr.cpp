
#include "stdafx.h"
#include "SceneMgr.h"
#include "Renderer.h"
#include "GlobalVal.h"

#include <random>


SceneMgr::SceneMgr() {
	m_Renderer = new Renderer(WindowWidth, WindowHeight);
	
	if (!m_Renderer->IsInitialized())
	{
		std::cout << "Renderer could not be initialized.. \n";
	}
	m_texImage[0] = m_Renderer->CreatePngTexture("Texture/sprite_2.png");
	m_texImage[1] = m_Renderer->CreatePngTexture("Texture/sprite_0.png");
	m_texImage[2] = m_Renderer->CreatePngTexture("Texture/sprite_1.png");

	//m_Sound = new Sound();
	//m_soundIndex[0] = m_Sound->CreateSound("./Dependencies/SoundSamples/ophelia.mp3");
	//
	//m_Sound->PlaySound(m_soundIndex[0], true, 0.2f);

}
SceneMgr::~SceneMgr() {
	//for (int i = 0; i < MAX_SOUND_COUNT; ++i) {
	//	m_Sound->DeleteSound(m_soundIndex[i]);
	//}
}


void SceneMgr::BuildObjects() 
{
	const int nRow = 400;
	m_mapTile = new Objects**[nRow];
	for (int i = 0; i < nRow; ++i) {
		m_mapTile[i] = new Objects*[400];
		for (int j = 0; j < 400; ++j) {
			float3 pos(i * tileSize - WindowWidth / 2 + tileSize / 2, j * tileSize - WindowHeight / 2 + tileSize / 2, 0);
			if ((i + j) % 2 == 0)
				m_mapTile[i][j] = AddObject(pos, float3(0, 0, 0), ObjectType::tile0, 0);
			else
				m_mapTile[i][j] = AddObject(pos, float3(0, 0, 0), ObjectType::tile1, 0);
			m_mapTile[i][j]->setLive(true);
		}
	}

	m_player = AddObject(float3(0, 0, 0), float3(0, 0, 0), ObjectType::player, m_texImage[0]);
	for (int i = 0; i < MAX_USER; ++i) {
		m_other[i] = AddObject(float3(0, 0, 0), float3(0, 0, 0), ObjectType::other, m_texImage[1]);
	}
	for (int i = 0; i < NUM_OF_NPC; ++i) {
		m_npc[i] = AddObject(float3(0, 0, 0), float3(0, 0, 0), ObjectType::npc, m_texImage[2]);
	}

	GlobalVal::getInstance()->SetObject(m_player, m_other, m_npc);
	GlobalVal::getInstance()->SetGlobalPos(&g_xPos, &g_yPos);
}

void SceneMgr::Update(float ElapsedTime) {
	DWORD currTime = timeGetTime() *0.001f;
}

void SceneMgr::Render() {
	for (int i = 0; i < 400; ++i) {
		for (int j = 0; j < 400; ++j) {
			int tile_x = j + g_xPos;
			int tile_y = i + g_yPos;
			if (tile_x < 0 || tile_y < 0) continue;
			if (tile_x < g_xPos || tile_y < g_yPos) continue;
			if (tile_x >= g_xPos+20 || tile_y >= g_yPos+20) continue;
			m_mapTile[tile_x][tile_y]->Render(*m_Renderer);
		}
	}

	for (int i = 0; i < NUM_OF_NPC; ++i)
		m_npc[i]->Render(*m_Renderer);
	for (int i = 0; i < MAX_USER; ++i)
		m_other[i]->Render(*m_Renderer);
	m_player->Render(*m_Renderer);
}

void SceneMgr::MouseInput(int x, int y) {
	if (y > 0)
		return;

}

Objects* SceneMgr::AddObject(float3 pos, float3 dir, int type, int texIndex) 
{
	Objects* newObject = nullptr;
	switch (type) {
	case ObjectType::tile0:
		newObject = new Objects(pos, float4(1, 1, 1, 1), 50.0f, 1);
		newObject->setRenderLevel(1.0f);
		break;
	case ObjectType::tile1:
		newObject = new Objects(pos, float4(0, 0, 0, 1), 50.0f, 1);
		newObject->setRenderLevel(1.0f);
		break;
	case ObjectType::player:
		newObject = new Objects(pos, float4(1, 1, 1, 1), 30.0f, 1);
		newObject->setTexIndex(texIndex);
		newObject->setRenderLevel(1.0f);
		break;
	case ObjectType::other:
		newObject = new Objects(pos, float4(1, 1, 1, 1), 30.0f, 1);
		newObject->setTexIndex(texIndex);
		newObject->setRenderLevel(1.0f);
		break;
	case ObjectType::npc:
		newObject = new Objects(pos, float4(1, 1, 1, 1), 30.0f, 1);
		newObject->setTexIndex(texIndex);
		newObject->setRenderLevel(1.0f);
		break;
	}
	return newObject;
}