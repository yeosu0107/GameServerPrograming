#pragma once

#include <vector>
#include "Objects.h"

class Renderer;

//OpenGL ����
#define  GLUT_KEY_LEFT            0x0064
#define  GLUT_KEY_UP               0x0065
#define  GLUT_KEY_RIGHT          0x0066
#define  GLUT_KEY_DOWN          0x0067

const float MoveValue = 62.0f;

class ObjectMgr
{
private:
	vector<Objects*> m_Objects;
	Objects* m_Player; //������ ��
public:
	ObjectMgr();
	~ObjectMgr();

	void BuildObjects(); //��� ������Ʈ ����

	void RenderObjects(Renderer& renderer); //����
	void UpdateObjects(); //������Ʈ ����

	void MovePlayer(int dir); //�÷��̾� �̵�
};

