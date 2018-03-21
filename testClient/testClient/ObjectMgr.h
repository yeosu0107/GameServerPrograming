#pragma once

#include <vector>
#include "Objects.h"

class Renderer;

//OpenGL ����
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
	//Objects* m_Player; //������ ��
	bool isRender[10];
public:
	ObjectMgr();
	~ObjectMgr();

	void BuildObjects(); //��� ������Ʈ ����

	void RenderObjects(Renderer& renderer); //����
	void UpdateObjects(); //������Ʈ ����

	void MovePlayer(int xPos, int yPos); //�÷��̾� �̵� for ����
	void MovePlayers(char* dataes);
};

