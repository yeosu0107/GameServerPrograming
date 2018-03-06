#pragma once

#include <vector>
#include "Objects.h"

class Renderer;

//OpenGL 변수
#define  GLUT_KEY_LEFT            0x0064
#define  GLUT_KEY_UP               0x0065
#define  GLUT_KEY_RIGHT          0x0066
#define  GLUT_KEY_DOWN          0x0067

const float MoveValue = 62.0f;

class ObjectMgr
{
private:
	vector<Objects*> m_Objects;
	Objects* m_Player; //움직일 말
public:
	ObjectMgr();
	~ObjectMgr();

	void BuildObjects(); //모든 오브젝트 빌드

	void RenderObjects(Renderer& renderer); //랜더
	void UpdateObjects(); //오브젝트 갱신

	void MovePlayer(int dir); //플레이어 이동
};

