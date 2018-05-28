/*
Copyright 2017 Lee Taek Hee (Korea Polytech University)

This program is free software: you can redistribute it and/or modify
it under the terms of the What The Hell License. Do it plz.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.
*/
#include "stdafx.h"

#include "Dependencies\glew.h"
#include "Dependencies\freeglut.h"

#include "GlobalVal.h"
#include "SceneMgr.h"
#include "Connect.h"



SceneMgr* scene = nullptr;
ServerConnect* server = nullptr;
bool g_LButtonDown = false;
int thisWindow = -1;

DWORD prevTime = timeGetTime();


void RenderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);
	glClearDepth(1.0f);

	DWORD currTime = timeGetTime();
	float time = (currTime - prevTime)*0.001f;
	prevTime = currTime;

	scene->Update(time);
	scene->Render();
	glutSwapBuffers();
}

void Idle(void)
{
	server->ReadPacket();
	RenderScene();
}

void MouseInput(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		g_LButtonDown = true;
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		if (g_LButtonDown) {
			int xPos = x - WindowWidth / 2;
			int yPos = WindowHeight / 2 - y;

			scene->MouseInput(xPos, yPos);

			g_LButtonDown = false;
		}
	}
}

void KeyInput(unsigned char key, int x, int y)
{
	
}

DWORD prevInputTime;
float inputInterval = 1.0f;

void SpecialKeyInput(int key, int x, int y)
{
	int tx = 0, ty = 0;
	float time = (timeGetTime() - prevInputTime)*0.001f;
	if (time < inputInterval)
		return;
	prevInputTime = timeGetTime();
	switch (key) {
	case GLUT_KEY_LEFT:
		tx = -1;
		break;
	case GLUT_KEY_RIGHT:
		tx = 1;
		break;
	case GLUT_KEY_UP:
		ty = -1;
		break;
	case GLUT_KEY_DOWN:
		ty = 1;
		break;
	}
	server->SendPacket(tx, ty);
}

int main(int argc, char **argv)
{
	server = new ServerConnect();
	// Initialize GL things
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WindowWidth, WindowHeight);
	thisWindow = glutCreateWindow("testClient");

	glewInit();
	if (glewIsSupported("GL_VERSION_3_0"))
	{
		std::cout << " GLEW Version is 3.0\n ";
	}
	else
	{
		std::cout << "GLEW 3.0 not supported\n ";
	}
	scene = new SceneMgr();
	scene->BuildObjects();
	
	glutDisplayFunc(RenderScene);
	glutIdleFunc(Idle);
	glutKeyboardFunc(KeyInput);
	glutMouseFunc(MouseInput);
	glutSpecialFunc(SpecialKeyInput);
	
	glutMainLoop();

	delete scene;
    return 0;
}

void CloseWindow() {
	glutDestroyWindow(thisWindow);
}