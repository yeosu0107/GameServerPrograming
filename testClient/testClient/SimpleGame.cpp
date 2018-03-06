/*
Game Software Engineering
*/

#include "stdafx.h"


#include "Dependencies\glew.h"
#include "Dependencies\freeglut.h"

#include "Renderer.h"
#include "ObjectMgr.h"

Renderer *g_Renderer = nullptr;
ObjectMgr* g_Mgr = nullptr;


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
	RenderScene();
}

void MouseInput(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		//마우스 좌표 보정
		int xPos = x - WindowWidth / 2;
		int yPos = WindowHeight / 2 - y;

		cout << xPos << "\t" << yPos << endl;
	}
}

void KeyInput(unsigned char key, int x, int y)
{

}

void SpecialKeyInput(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_RIGHT:
	case GLUT_KEY_UP:
	case GLUT_KEY_DOWN:
	case GLUT_KEY_LEFT:
		g_Mgr->MovePlayer(key);
		break;

	default:
		break;
	}
}

int main(int argc, char **argv)
{
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

	//랜더러 초기화
	g_Renderer = new Renderer(WindowWidth, WindowHeight);
	if (!g_Renderer->IsInitialized())
		cout << "Renderer 생성 실패 \n";
	//오브젝트 매니저 초기화
	g_Mgr = new ObjectMgr();
	if (!g_Mgr)
		cout << "ObjectMgr 생성 실패\n" << endl;

	glutDisplayFunc(RenderScene);
	glutIdleFunc(Idle);
	glutKeyboardFunc(KeyInput);
	glutMouseFunc(MouseInput);
	glutSpecialFunc(SpecialKeyInput);

	glutMainLoop();

	delete g_Renderer;
	delete g_Mgr;
    return 0;
}

