#include "stdafx.h"
#include "ObjectMgr.h"

float4 color[10] = {
	float4(1,0,0,1),
	float4(1,1,0,1),
	float4(1,1,1,1),
	float4(0,1,0,1),
	float4(0,0,1,1),
	float4(0,1,1,1),
	float4(0,0,0,1),
	float4(0.5f, 0.5f, 0,1),
	float4(0,0.5f, 0.5f, 1),
	float4(0.5f,0 , 0.5f, 1)
};

ObjectMgr::ObjectMgr()
{
	BuildObjects();
}


ObjectMgr::~ObjectMgr()
{
	m_Objects.clear();
}

void ObjectMgr::BuildObjects()
{
	float xPos = -WindowHeight / 2;
	float yPos = -WindowWidth / 2;
	int intervalX = WindowWidth / 8;
	int intervalY = WindowHeight / 8;

	float playerSize = 30.0f;

	Objects* lineObject = nullptr; //바둑판 선

	for (int i = 0; i < 9; ++i) {
		//가로줄
		float ty = yPos + intervalY * i;
		lineObject = new Objects(float3(0.0f, ty, 0.0f), float4(1, 1, 1, 1), WindowWidth, 10.0f, 0, "Line", float3(0, 0, 0), 0.0f);
		m_Objects.emplace_back(lineObject);

		//세로줄
		float tx = xPos + intervalX * i;
		lineObject = new Objects(float3(tx, 0.0f, 0.0f), float4(1, 1, 1, 1), 10.0f, WindowHeight, 0, "Line", float3(0, 0, 0), 0.0f);
		m_Objects.emplace_back(lineObject);
	}

	for (int i = 0; i < 10; ++i) {
		Objects* player= new Objects(float3(playerStartX, playerStartY, 0), color[i], playerSize, playerSize,
			1.0f, "Player", float3(0, 0, 0), 0.0f);
		player->setLive(false);

		m_Player.emplace_back(player);
	}
	//m_Objects.emplace_back(m_Player);
}

void ObjectMgr::RenderObjects(Renderer & renderer)
{
	for (Objects* object : m_Objects) {
		object->Update();
		object->Render(renderer);
	}
	int i = 0;
	for (Objects* player : m_Player) {
		if (isRender[i]) {
			player->Update();
			player->Render(renderer);
		}
		i += 1;
	}
}

void ObjectMgr::UpdateObjects()
{
	for (Objects* object : m_Objects) {
		object->Update();
	}
}


void ObjectMgr::MovePlayer(int xPos, int yPos)
{
	int applyX = playerStartX + (xPos * MoveValue);
	int applyY = playerStartY +  (yPos * -MoveValue);
	m_Player[0]->Move(applyX, applyY);
}

void ObjectMgr::MovePlayers(char * dataes)
{
	memcpy(playerInfo, dataes, sizeof(PlayerInfo)*clientNum);
	memset(isRender, false, sizeof(bool) * 10);

	for (int i = 0; i < clientNum; ++i) {
		int applyX = playerStartX + (playerInfo[i].xPos * MoveValue);
		int applyY = playerStartY + (playerInfo[i].yPos * -MoveValue);
		m_Player[playerInfo[i].m_playerNum]->Move(applyX, applyY);
		isRender[playerInfo[i].m_playerNum] = true;
	}
	
}
