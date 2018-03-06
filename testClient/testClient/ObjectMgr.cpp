#include "stdafx.h"
#include "ObjectMgr.h"


ObjectMgr::ObjectMgr() : m_Player(nullptr)
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
	m_Player = new Objects(float3(-217.5f, 217.5f, 0), float4(1, 0, 0, 1), playerSize, playerSize,
		1.0f, "Player", float3(0, 0, 0), 0.0f);
	m_Objects.emplace_back(m_Player);
}

void ObjectMgr::RenderObjects(Renderer & renderer)
{
	for (Objects* object : m_Objects) {
		object->Update();
		object->Render(renderer);
	}
}

void ObjectMgr::UpdateObjects()
{
	for (Objects* object : m_Objects) {
		object->Update();
	}
}

void ObjectMgr::MovePlayer(int dir)
{
	float3 check = m_Player->getPos();
	float3 moveDir(0,0,0);
	switch (dir) {
	case GLUT_KEY_RIGHT:
		moveDir.x = MoveValue;
		check += moveDir;
		if(WindowWidth/2 > check.x)
			m_Player->Move(moveDir);
		break;
	case GLUT_KEY_UP:
		moveDir.y = MoveValue;
		check += moveDir;
		if (WindowHeight /2 > check.y)
			m_Player->Move(moveDir);
		break;
	case GLUT_KEY_DOWN:
		moveDir.y = -MoveValue;
		check += moveDir;
		if (-WindowHeight / 2 < check.y)
			m_Player->Move(moveDir);
		break;
	case GLUT_KEY_LEFT:
		moveDir.x = -MoveValue;
		check += moveDir;
		if (-WindowWidth/2 < check.x)
			m_Player->Move(moveDir);
		break;
	default:
		break;
	}
}
