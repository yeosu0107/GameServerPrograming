/*
Game Software Engineering
*/

#include "stdafx.h"


#include "Dependencies\glew.h"
#include "Dependencies\freeglut.h"

#include "Renderer.h"
#include "ObjectMgr.h"
#include "ServerConnect.h"

Renderer *g_Renderer = nullptr;
ObjectMgr* g_Mgr = nullptr;
ServerConnect* g_server = nullptr;
char* checkData = nullptr;
int retval = -1;

void shutDown()
{
	cout << "�������� ������ ������ϴ�" << endl;
	cout << "���α׷��� �����մϴ�" << endl;
	glutLeaveMainLoop();
}

void RenderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	
	g_Mgr->RenderObjects(*g_Renderer);

	glutSwapBuffers();
}

void Idle(void)
{
	g_Mgr->UpdateObjects();
	retval = g_server->RecvData();
		
	RenderScene();
	retval = g_server->SendData();
	if (retval == SOCKET_ERROR) {
		shutDown();
	}
		
	if (checkData[0] != -1) {
		g_Mgr->MovePlayer(checkData[0], checkData[1]);
	}
}

void MouseInput(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		////���콺 ��ǥ ����
		//int xPos = x - WindowWidth / 2;
		//int yPos = WindowHeight / 2 - y;

		////cout << xPos << "\t" << yPos << endl;
	}
}

void KeyInput(unsigned char key, int x, int y)
{

}

void SpecialKeyInput(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_RIGHT:
		g_server->getKeyData() |= DIR_RIGHT;
		break;
	case GLUT_KEY_UP:
		g_server->getKeyData() |= DIR_UP;
		break;
	case GLUT_KEY_DOWN:
		g_server->getKeyData() |= DIR_DOWN;
		break;
	case GLUT_KEY_LEFT:
		g_server->getKeyData() |= DIR_LEFT;
		break;
		g_Mgr->MovePlayer(key);
	default:
		break;
	}
	
	//cout << g_server->getKeyData() << endl;
}

int main(int argc, char **argv)
{
	//������ ����
	g_server = new ServerConnect();

	// Initialize GL things
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutCreateWindow("GameServer Test Client");

	glewInit();
	if (glewIsSupported("GL_VERSION_3_0"))
	{
		std::cout << " GLEW Version is 3.0\n ";
	}
	else
	{
		std::cout << "GLEW 3.0 not supported\n ";
	}


	//������ �ʱ�ȭ
	g_Renderer = new Renderer(WindowWidth, WindowHeight);
	if (!g_Renderer->IsInitialized())
		cout << "Renderer ���� ����" << endl;;
	//������Ʈ �Ŵ��� �ʱ�ȭ
	g_Mgr = new ObjectMgr();
	if (!g_Mgr)
		cout << "ObjectMgr ���� ����" << endl;
	
	checkData = g_server->getRecvData();
	glutDisplayFunc(RenderScene);
	glutIdleFunc(Idle);
	glutKeyboardFunc(KeyInput);
	glutMouseFunc(MouseInput);
	glutSpecialFunc(SpecialKeyInput);

	glutMainLoop();

	delete g_server;
	delete g_Renderer;
	delete g_Mgr;
    return 0;
}

