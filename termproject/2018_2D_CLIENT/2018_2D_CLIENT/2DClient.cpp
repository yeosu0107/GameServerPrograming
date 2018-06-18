// PROG14_1_16b.CPP - DirectInput keyboard demo

// INCLUDES ///////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN  
#define INITGUID

#pragma comment(linker,"/entry:WinMainCRTStartup /subsystem:console")


#include <WinSock2.h>
#include <windows.h>   // include important windows stuff
#include <windowsx.h>
#include <iostream>
#include <mmsystem.h>


#include <d3d9.h>     // directX includes
#include "d3dx9tex.h"     // directX includes
#include "gpdumb1.h"
#include "mapObject.h"
#include "Effect.h"
#include "textManager.h"
#include "monster.h"

#include "..\..\2018Server\2018Server\protocol.h"

#pragma comment (lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib") 

using namespace std;

// DEFINES ////////////////////////////////////////////////

#define MAX(a,b)	((a)>(b))?(a):(b)
#define	MIN(a,b)	((a)<(b))?(a):(b)

// defines for windows 
#define WINDOW_CLASS_NAME L"WINXCLASS"  // class name

#define WINDOW_WIDTH    680  // size of window 680
#define WINDOW_HEIGHT   710

#define	BUF_SIZE				1024
#define	WM_SOCKET				WM_USER + 1

// PROTOTYPES /////////////////////////////////////////////

// game console
int Game_Init(void *parms = NULL);
int Game_Shutdown(void *parms = NULL);
int Game_Main(void *parms = NULL);

// GLOBALS ////////////////////////////////////////////////

HWND main_window_handle = NULL; // save the window handle
HINSTANCE main_instance = NULL; // save the instance
char buffer[80];                // used to print text

wchar_t g_message[300];
unsigned char color_r = 255;
unsigned char color_g = 255;
unsigned char color_b = 255;

// demo globals
BOB			player;				// 플레이어 Unit
bool			player_animate = false;
//BOB			npc[NUM_OF_NPC];      // NPC Unit
Monster		monster[NUM_OF_NPC];
BOB         skelaton[MAX_USER];     // the other player skelaton

MapObject* mapObject;
Effect* attackEffect;
SkillEffect* skillEffect1;
SkillEffect* skillEffect2;
SkillEffect* bossEffect[5];
int e_index = 0;
textManager* logMsg;

#define TILE_WIDTH 32

#define UNIT_TEXTURE  0
#define UINT_TREE_TEXTURE 1
#define MAP_BACKTEXTURE 2
#define EXPLOSION_TEXTURE 3
#define MONSTER_TEXTURE 4
#define CHARACTER_TEXTURE 5
#define SKILL1_TEXTURE 6
#define SKILL2_TEXTURE 7
#define BOSS_SKILL1_TEXTURE 8
#define BOSS_SKILL2_TEXTURE 9

SOCKET g_mysocket;
WSABUF	send_wsabuf;
char 	send_buffer[BUF_SIZE];
WSABUF	recv_wsabuf;
char	recv_buffer[BUF_SIZE];
char	packet_buffer[BUF_SIZE];
DWORD		in_packet_size = 0;
int		saved_packet_size = 0;
int		g_myid;

int g_myexp = 0;
int g_maxexp = 0;
int g_myhp = 0;
int g_mylevel = 0;

int		g_left_x = 0;
int     g_top_y = 0;


// FUNCTIONS //////////////////////////////////////////////
void ProcessPacket(char *ptr)
{
	static bool first_time = true;
	switch (ptr[1])
	{
	case SC_PUT_PLAYER:
	{
		sc_packet_put_player *my_packet = reinterpret_cast<sc_packet_put_player *>(ptr);
		int id = my_packet->id;
		if (first_time) {
			first_time = false;
			g_myid = id;
		}
		if (id == g_myid) {
			g_left_x = my_packet->x - 10;
			g_top_y = my_packet->y - 10;
			player.x = my_packet->x;
			player.y = my_packet->y;
			player.attr |= BOB_ATTR_VISIBLE;
		}
		else if (id < NPC_START) {
			skelaton[id].x = my_packet->x;
			skelaton[id].y = my_packet->y;
			skelaton[id].attr |= BOB_ATTR_VISIBLE;
		}
		else {
			monster[id - NPC_START].Initialize(my_packet->x, my_packet->y);
			if (monster[id - NPC_START].type != my_packet->pic_type) {
				monster[id - NPC_START].type = my_packet->pic_type;
				Load_Frame_BOB32(&monster[id - NPC_START].object, MONSTER_TEXTURE, 0, my_packet->pic_type * 3, 0, BITMAP_EXTRACT_MODE_CELL);
				Load_Frame_BOB32(&monster[id - NPC_START].object, MONSTER_TEXTURE, 1, my_packet->pic_type * 3 + 1, 0, BITMAP_EXTRACT_MODE_CELL);
				Load_Frame_BOB32(&monster[id - NPC_START].object, MONSTER_TEXTURE, 2, my_packet->pic_type * 3 + 2, 0, BITMAP_EXTRACT_MODE_CELL);
			}
			//npc[id - NPC_START].x = my_packet->x;
			//npc[id - NPC_START].y = my_packet->y;
			//npc[id - NPC_START].attr |= BOB_ATTR_VISIBLE;
		}
		break;
	}
	case SC_POS:
	{
		sc_packet_pos *my_packet = reinterpret_cast<sc_packet_pos *>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			g_left_x = my_packet->x - 10;
			g_top_y = my_packet->y - 10;
			if (player.x > my_packet->x) {
				if(player.curr_animation != 1)
					Set_Animation_BOB32(&player, 1);
			}
			else if (player.x < my_packet->x) {
				if (player.curr_animation != 2)
					Set_Animation_BOB32(&player, 2);
			}
			else if (player.y > my_packet->y) {
				if (player.curr_animation != 3)
					Set_Animation_BOB32(&player, 3);
			}
			else if (player.y < my_packet->y) {
				if (player.curr_animation != 0)
					Set_Animation_BOB32(&player, 0);
			}

			player.x = my_packet->x;
			player.y = my_packet->y;

			player_animate = true;

		}
		else if (other_id < NPC_START) {
			skelaton[other_id].x = my_packet->x;
			skelaton[other_id].y = my_packet->y;
		}
		else {
			monster[other_id - NPC_START].setPos(my_packet->x, my_packet->y);
			//npc[other_id - NPC_START].x = my_packet->x;
			//npc[other_id - NPC_START].y = my_packet->y;
		}
		break;
	}

	case SC_REMOVE_PLAYER:
	{
		sc_packet_remove_player *my_packet = reinterpret_cast<sc_packet_remove_player *>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			player.attr &= ~BOB_ATTR_VISIBLE;
		}
		else if (other_id < NPC_START) {
			skelaton[other_id].attr &= ~BOB_ATTR_VISIBLE;
		}
		else {
			monster[other_id - NPC_START].object.attr &= ~BOB_ATTR_VISIBLE;
			//npc[other_id - NPC_START].attr &= ~BOB_ATTR_VISIBLE;
		}
		break;
	}
	case SC_CHAT:
	{
		sc_packet_chat *my_packet = reinterpret_cast<sc_packet_chat *>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			//wcsncpy_s(player.message, my_packet->message, 256);
			//player.message_time = GetTickCount();
			wcsncpy_s(g_message, my_packet->message, 256);
			logMsg->insert(GetTickCount(), g_message);
			//color_r = 0;
			//color_g = 255;
			//color_b = 0;
		}
		else if (other_id < NPC_START) {
			//wcsncpy_s(skelaton[other_id].message, my_packet->message, 256);
			//skelaton[other_id].message_time = GetTickCount();
		}
		else {
			//wcsncpy_s(npc[other_id - NPC_START].message, my_packet->message, 256);
			//npc[other_id - NPC_START].message_time = GetTickCount();
			wcsncpy_s(monster[other_id - NPC_START].object.message, my_packet->message, 256);
			monster[other_id - NPC_START].object.message_time = GetTickCount();
			if(my_packet->info == INFO_ATTACK)
				monster[other_id - NPC_START].setDir(player.x, player.y);
		}
		break;
	}
	case SC_DUPLICATON_PLAYER:
		std::cout << "이미 접속한 ID입니다" << endl;
		std::cout << "접속을 종료합니다" << endl;
		exit(0);
		break;
	case SC_PLAYER_STAT:
	{
		sc_packet_stat * myPacket = reinterpret_cast<sc_packet_stat*>(ptr);
		g_myexp = myPacket->exp;
		g_mylevel = myPacket->level;
		g_myhp = myPacket->hp;
		g_maxexp = myPacket->max_exp;
		break;
	}
	case SC_PLAYER_SKILL:
	{
		sc_packet_skill* myPacket = reinterpret_cast<sc_packet_skill*>(ptr);
		if (myPacket->kind == 0) {
			attackEffect->update(player.x, player.y);
		}
		else if (myPacket->kind == 1) {
			skillEffect1->update(player.x, player.y);
		}
		else if (myPacket->kind == 2) {
			skillEffect2->update(player.x, player.y);
		}
	}
	break;
	case SC_BOSS_SKILL:
	{
		sc_packet_boss* myPacket = reinterpret_cast<sc_packet_boss*>(ptr);
		if (myPacket->kind == 0) {
			bossEffect[e_index]->update(myPacket->x, myPacket->y);
			e_index += 1;
			if (e_index >= 4)
				e_index = 0;
		}
		else if (myPacket->kind == 1) {
			bossEffect[4]->update(myPacket->x, myPacket->y);
		}
	}
		break;
	default:
		printf("Unknown PACKET type [%d]\n", ptr[1]);
	}
}

void ReadPacket(SOCKET sock)
{
	DWORD iobyte, ioflag = 0;

	int ret = WSARecv(sock, &recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
	if (ret) {
		int err_code = WSAGetLastError();
		printf("Recv Error [%d]\n", err_code);
	}

	BYTE *ptr = reinterpret_cast<BYTE *>(recv_buffer);

	while (0 != iobyte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (iobyte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			iobyte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, iobyte);
			saved_packet_size += iobyte;
			iobyte = 0;
		}
	}
}

void clienterror()
{
	exit(-1);
}

DWORD prevInputTime;
float inputInterval = 0.1f;

LRESULT CALLBACK WindowProc(HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam)
{
	// this is the main message handler of the system
	PAINTSTRUCT	ps;		   // used in WM_PAINT
	HDC			hdc;	   // handle to a device context

	// what is the message 
	switch (msg)
	{
	case WM_KEYDOWN: {
		float time = (timeGetTime() - prevInputTime)*0.001f;
		if (time < inputInterval)
			break;
		prevInputTime = timeGetTime();
		int x = 0, y = 0;
		bool attack = false;
		bool skill1 = false;
		bool skill2 = false;
		if (wparam == VK_RIGHT)	x += 1;
		if (wparam == VK_LEFT)	x -= 1;
		if (wparam == VK_UP)	y -= 1;
		if (wparam == VK_DOWN)	y += 1;
		if (wparam == VK_SPACE) attack = true;
		if (wparam == 81) skill1 = true; //q
		if (wparam == 69) skill2 = true; //e
		cs_packet_up *my_packet = reinterpret_cast<cs_packet_up *>(send_buffer);
		my_packet->size = sizeof(my_packet);
		send_wsabuf.len = sizeof(my_packet);
		DWORD iobyte;
		if(attack == true){
			my_packet->type = CS_ATTACK;
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
			//attackEffect->update(player.x, player.y);
		}
		else if (skill1 == true) {
			my_packet->type = CS_SKILL1;
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
			//skillEffect1->update(player.x, player.y);
		}
		else if (skill2 == true) {
			my_packet->type = CS_SKILL2;
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
			//skillEffect2->update(player.x, player.y);
		}
		else {
			//이동 이벤트
			if (0 != x) {
				if (1 == x) my_packet->type = CS_RIGHT;
				else my_packet->type = CS_LEFT;
				WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
				/*if (ret) {
				int error_code = WSAGetLastError();
				printf("Error while sending packet [%d]", error_code);
				}*/
			}
			if (0 != y) {
				if (1 == y) my_packet->type = CS_DOWN;
				else my_packet->type = CS_UP;
				WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
			}
		}


	}
	break;
	case WM_CREATE:
	{
		// do initialization stuff here
		return(0);
	} break;

	case WM_PAINT:
	{
		// start painting
		hdc = BeginPaint(hwnd, &ps);

		// end painting
		EndPaint(hwnd, &ps);
		return(0);
	} break;

	case WM_DESTROY:
	{
		// kill the application			
		PostQuitMessage(0);
		return(0);
	} break;
	case WM_SOCKET:
	{
		if (WSAGETSELECTERROR(lparam)) {
			closesocket((SOCKET)wparam);
			clienterror();
			break;
		}
		switch (WSAGETSELECTEVENT(lparam)) {
		case FD_READ:
			ReadPacket((SOCKET)wparam);
			break;
		case FD_CLOSE:
			closesocket((SOCKET)wparam);
			clienterror();
			break;
		}
	}

	default:
		break;
	} // end switch

// process any messages that we didn't take care of 
	return (DefWindowProc(hwnd, msg, wparam, lparam));

}// end WinProc

// WINMAIN ////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE hprevinstance,
	LPSTR lpcmdline,
	int ncmdshow)
{
	// this is the winmain function

	WNDCLASS winclass;	// this will hold the class we create
	HWND	 hwnd;		// generic window handle
	MSG		 msg;		// generic message


	// first fill in the window class stucture
	winclass.style = CS_DBLCLKS | CS_OWNDC |
		CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc = WindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = hinstance;
	winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = WINDOW_CLASS_NAME;

	// register the window class
	if (!RegisterClass(&winclass))
		return(0);

	// create the window, note the use of WS_POPUP
	if (!(hwnd = CreateWindow(WINDOW_CLASS_NAME, // class
		L"Chess Client",	 // title
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		0, 0,	   // x,y
		WINDOW_WIDTH,  // width
		WINDOW_HEIGHT, // height
		NULL,	   // handle to parent 
		NULL,	   // handle to menu
		hinstance,// instance
		NULL)))	// creation parms
		return(0);

	// save the window handle and instance in a global
	main_window_handle = hwnd;
	main_instance = hinstance;

	// perform all game console specific initialization
	Game_Init();

	// enter main event loop
	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// test if this is a quit
			if (msg.message == WM_QUIT)
				break;

			// translate any accelerator keys
			TranslateMessage(&msg);

			// send the message to the window proc
			DispatchMessage(&msg);
		} // end if

	// main game processing goes here
		Game_Main();

	} // end while

// shutdown game and release all resources
	Game_Shutdown();

	// return to Windows like this
	return(msg.wParam);

} // end WinMain

///////////////////////////////////////////////////////////

// WINX GAME PROGRAMMING CONSOLE FUNCTIONS ////////////////

int Game_Init(void *parms)
{
	// this function is where you do all the initialization 
	// for your game
	logMsg = new textManager();
	// set up screen dimensions
	screen_width = WINDOW_WIDTH;
	screen_height = WINDOW_HEIGHT;
	screen_bpp = 32;

	// initialize directdraw
	DD_Init(screen_width, screen_height, screen_bpp);


	// now let's load in all the frames for the skelaton!!!

	Load_Texture(L"CHESS2.PNG", UNIT_TEXTURE, 192, TILE_WIDTH);
	Load_Texture(L"monsterSet.png", MONSTER_TEXTURE, 384, 256);
	Load_Texture(L"myMap.png", MAP_BACKTEXTURE, 9600, 9600);
	Load_Texture(L"explosion.png", EXPLOSION_TEXTURE, 240, 41);
	Load_Texture(L"charSet.png", CHARACTER_TEXTURE, 384, 256);
	Load_Texture(L"skill1.png", SKILL1_TEXTURE, 640, 896);
	Load_Texture(L"skill2.png", SKILL2_TEXTURE, 640, 640);
	Load_Texture(L"boss_skill1.png", BOSS_SKILL1_TEXTURE, 640, 768);
	Load_Texture(L"boss_skill2.png", BOSS_SKILL2_TEXTURE, 640, 768);


	if (!Create_BOB32(&player, 0, 0, TILE_WIDTH, TILE_WIDTH, 12, BOB_ATTR_MULTI_ANIM)) return(0);
	
	int index = 0;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 3; ++j) {
			Load_Frame_BOB32(&player, CHARACTER_TEXTURE, index, j, i, BITMAP_EXTRACT_MODE_CELL);
			index += 1;
		}
	}
	int* down = new int[3]{ 0,1,2 };
	int* left = new int[3]{ 3,4,5 };
	int* right = new int[3]{ 6,7,8 };
	int* up = new int[3]{ 9,10,11 };

	Load_Animation_BOB32(&player, 0, 3, down);
	Load_Animation_BOB32(&player, 1, 3, left);
	Load_Animation_BOB32(&player, 2, 3, right);
	Load_Animation_BOB32(&player, 3, 3, up);

	// set up stating state of skelaton
	Set_Animation_BOB32(&player, 0);
	Set_Anim_Speed_BOB32(&player, 1);
	Set_Vel_BOB32(&player, 0, 0);
	Set_Pos_BOB32(&player, 0, 0);

	mapObject = new MapObject(MAP_BACKTEXTURE);
	attackEffect = new Effect(EXPLOSION_TEXTURE, 40, 41, 6);
	skillEffect1 = new SkillEffect(SKILL1_TEXTURE, 128, 128, 5, 7);
	skillEffect2 = new SkillEffect(SKILL2_TEXTURE, 128	, 128, 5, 5);
	for (int i = 0; i < 4; ++i)
		bossEffect[i] = new SkillEffect(BOSS_SKILL1_TEXTURE, 128, 128, 5, 6);
	bossEffect[4] = new SkillEffect(BOSS_SKILL2_TEXTURE, 128, 128, 5, 6);

	// create skelaton bob
	for (int i = 0; i < MAX_USER; ++i) {
		if (!Create_BOB32(&skelaton[i], 0, 0, TILE_WIDTH, TILE_WIDTH, 1, BOB_ATTR_SINGLE_FRAME))
			return(0);
		Load_Frame_BOB32(&skelaton[i], UNIT_TEXTURE, 0, 0, 0, BITMAP_EXTRACT_MODE_CELL);

		// set up stating state of skelaton
		Set_Animation_BOB32(&skelaton[i], 0);
		Set_Anim_Speed_BOB32(&skelaton[i], 4);
		Set_Vel_BOB32(&skelaton[i], 0, 0);
		Set_Pos_BOB32(&skelaton[i], 0, 0);
	}

	// create skelaton bob
	for (int i = 0; i < NUM_OF_NPC; ++i) {
		BOB* npc = &monster[i].object;
		if (!Create_BOB32(npc, 0, 0, TILE_WIDTH, TILE_WIDTH, 3, BOB_ATTR_MULTI_FRAME))
			return(0);
		//Load_Frame_BOB32(npc, MONSTER_TEXTURE, 0, 0, 0, BITMAP_EXTRACT_MODE_CELL);

		// set up stating state of skelaton
		Set_Animation_BOB32(npc, 0);
		Set_Anim_Speed_BOB32(npc, 8);
		Set_Vel_BOB32(npc, 0, 0);
		Set_Pos_BOB32(npc, 0, 0);
	}



	// set clipping rectangle to screen extents so mouse cursor
	// doens't mess up at edges
	//RECT screen_rect = {0,0,screen_width,screen_height};
	//lpddclipper = DD_Attach_Clipper(lpddsback,1,&screen_rect);

	// hide the mouse
	//ShowCursor(FALSE);
	char ipAddress[20];
	int id;
#ifdef DB
	printf("set DB Connect : true\n");
	printf("로그인 정보\n");
	std::cout << "IP 입력 : ";
	std::cin >> ipAddress;
	std::cout << "ID 입력 : ";
	std::cin >> id;
#else
	strcpy(ipAddress, "127.0.0.1");
	id = 1;

#endif

	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	g_mysocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(MY_SERVER_PORT);
	ServerAddr.sin_addr.s_addr = inet_addr(ipAddress);

	int Result = WSAConnect(g_mysocket, (sockaddr *)&ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL);

	WSAAsyncSelect(g_mysocket, main_window_handle, WM_SOCKET, FD_CLOSE | FD_READ);

	send_wsabuf.buf = send_buffer;
	send_wsabuf.len = BUF_SIZE;
	recv_wsabuf.buf = recv_buffer;
	recv_wsabuf.len = BUF_SIZE;

	sc_packet_login *my_packet = reinterpret_cast<sc_packet_login *>(send_buffer);
	my_packet->size = sizeof(my_packet);
	my_packet->type = id;
	my_packet->id = id;
	send_wsabuf.len = sizeof(my_packet);

	DWORD iobyte;
	
	WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	// return success
	return(1);

} // end Game_Init

///////////////////////////////////////////////////////////

int Game_Shutdown(void *parms)
{
	// this function is where you shutdown your game and
	// release all resources that you allocated

	// kill the reactor

	// kill skelaton
	for (int i = 0; i < MAX_USER; ++i) Destroy_BOB32(&skelaton[i]);
	//for (int i = 0; i < NUM_OF_NPC; ++i)
		//Destroy_BOB32(&npc[i]);

	// shutdonw directdraw
	DD_Shutdown();

	WSACleanup();

	// return success
	return(1);
} // end Game_Shutdown

///////////////////////////////////////////////////////////

int Game_Main(void *parms)
{
	// this is the workhorse of your game it will be called
	// continuously in real-time this is like main() in C
	// all the calls for you game go here!
	// check of user is trying to exit
	if (KEY_DOWN(VK_ESCAPE)/* || KEY_DOWN(VK_SPACE)*/)
		PostMessage(main_window_handle, WM_DESTROY, 0, 0);
	mapObject->setRect(g_left_x, g_top_y);
	// start the timing clock
	Start_Clock();

	// clear the drawing surface
	DD_Fill_Surface(D3DCOLOR_ARGB(255, 0, 0, 0));

	// get player input

	g_pd3dDevice->BeginScene();
	g_pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE);
	
	mapObject->draw();
	
	g_pSprite->End();
	g_pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE);


	// draw the skelaton
	if (player_animate) {
		Animate_BOB32(&player);
		player_animate = false;
	}
	Draw_BOB32(&player);
	for (int i = 0; i < MAX_USER; ++i) Draw_BOB32(&skelaton[i]);
	for (int i = 0; i < NUM_OF_NPC; ++i) {
		monster[i].draw();
		//Draw_BOB32(&npc[i]);
	}
	
	//drawEffect
	if (attackEffect->now_render)
		attackEffect->draw();
	if (skillEffect1->now_render)
		skillEffect1->draw();
	if (skillEffect2->now_render)
		skillEffect2->draw();
	for (int i = 0; i < 5; ++i) {
		if (bossEffect[i]->now_render)
			bossEffect[i]->draw();
	}

	// draw some text
	wchar_t text[300];
	wsprintf(text, L"Level : %2d HP : %3d exp : %3d / %3d  POS (%3d, %3d)", g_mylevel, g_myhp, g_myexp, g_maxexp, player.x, player.y);
	Draw_Text_D3D(text, 10, screen_height - 64, D3DCOLOR_ARGB(255, 255, 255, 255));
	//Draw_Text_D3D(g_message, 10, screen_height - 128, D3DCOLOR_ARGB(255, color_r, color_g, color_b));
	logMsg->draw();
	g_pSprite->End();

	g_pd3dDevice->EndScene();

	// flip the surfaces
	DD_Flip();

	// sync to 3o fps
	//Wait_Clock(30);

	// return success
	return(1);

} // end Game_Main

//////////////////////////////////////////////////////////