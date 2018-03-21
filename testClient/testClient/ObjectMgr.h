#pragma once

#include <vector>
#include "Objects.h"

class Renderer;

//OpenGL 변수
#define  GLUT_KEY_LEFT            0x0064
#define  GLUT_KEY_UP               0x0065
#define  GLUT_KEY_RIGHT          0x0066
#define  GLUT_KEY_DOWN          0x0067

const float playerStartX =	-217.5f;
const float playerStartY =	217.5f;

const float MoveValue = 62.0f;

class ObjectMgr
{
private:
	vector<Objects*> m_Objects;
	vector<Objects*> m_Player;
	//Objects* m_Player; //움직일 말
	bool isRender[10];
public:
	ObjectMgr();
	~ObjectMgr();

	void BuildObjects(); //모든 오브젝트 빌드

	void RenderObjects(Renderer& renderer); //랜더
	void UpdateObjects(); //오브젝트 갱신

	void MovePlayer(int xPos, int yPos); //플레이어 이동 for 서버
	void MovePlayers(char* dataes);
};

