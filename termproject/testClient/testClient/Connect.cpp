#include "stdafx.h"
#include "Connect.h"
#include "GlobalVal.h"
#include "Objects.h"

void error_quit(char* msg)
{
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);

	LocalFree(lpMsgBuf);
	exit(1);
}

ServerConnect::ServerConnect()
{
	int id = InputID();

	if (WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0)
		return;

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == INVALID_SOCKET)
		error_quit("socket()");

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(MY_SERVER_PORT);
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int Result = connect(m_socket, (sockaddr *)&ServerAddr, sizeof(ServerAddr));

	send_wsabuf.buf = send_buffer;
	send_wsabuf.len = BUF_SIZE;
	recv_wsabuf.buf = recv_buffer;
	recv_wsabuf.len = BUF_SIZE;

	cs_packet_up *my_packet = reinterpret_cast<cs_packet_up *>(send_buffer);
	my_packet->size = sizeof(my_packet);
	send_wsabuf.len = sizeof(my_packet);
	DWORD iobyte;
	my_packet->type = id;
	WSASend(m_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
}

ServerConnect::~ServerConnect()
{
	closesocket(m_socket);
	WSACleanup();
}

void ServerConnect::ProcessPacket(char * ptr)
{
	static bool first_time = true;
	
	Objects* player = GlobalVal::getInstance()->getPlayer();
	Objects** skelaton = GlobalVal::getInstance()->getOther();
	Objects** npc = GlobalVal::getInstance()->getNpc();
	int* g_left_x = GlobalVal::getInstance()->getxPos();
	int* g_top_y = GlobalVal::getInstance()->getyPos();

	switch (ptr[1])
	{
	case SC_PUT_PLAYER:
	{
		sc_packet_put_player *my_packet = reinterpret_cast<sc_packet_put_player *>(ptr);
		float x = my_packet->x * tileSize - WindowWidth / 2 + tileSize / 2;
		float y = my_packet->y * tileSize - WindowHeight / 2 + tileSize / 2;
		int id = my_packet->id;
		if (first_time) {
			first_time = false;
			g_myid = id;
		}
		if (id == g_myid) {
			*g_left_x = my_packet->x - 10;
			*g_top_y = my_packet->y - 10;
			player->setPos(x, y, 0);
			player->setLive(true);
		}
		else if (id < NPC_START) {
			skelaton[id]->setPos(x, y, 0);
			skelaton[id]->setLive(true);
		}
		else {
			npc[id - NPC_START]->setPos(x, y, 0);
			npc[id - NPC_START]->setLive(true);
		}
		break;
	}
	case SC_POS:
	{
		sc_packet_pos *my_packet = reinterpret_cast<sc_packet_pos *>(ptr);
		float x = my_packet->x * tileSize - WindowWidth / 2 + tileSize / 2;
		float y = my_packet->y * tileSize - WindowHeight / 2 + tileSize / 2;
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			*g_left_x = my_packet->x - 10;
			*g_top_y = my_packet->y - 10;
			player->setPos(x, y, 0);
		}
		else if (other_id < NPC_START) {
			skelaton[other_id]->setPos(x, y, 0);
		}
		else {
			npc[other_id - NPC_START]->setPos(x, y, 0);
		}
		break;
	}

	case SC_REMOVE_PLAYER:
	{
		sc_packet_remove_player *my_packet = reinterpret_cast<sc_packet_remove_player *>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			player->setLive(false);
		}
		else if (other_id < NPC_START) {
			skelaton[other_id]->setLive(false);
		}
		else {
			npc[other_id - NPC_START]->setLive(false);
		}
		break;
	}
	/*case SC_CHAT:
	{
		sc_packet_chat *my_packet = reinterpret_cast<sc_packet_chat *>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			wcsncpy_s(player.message, my_packet->message, 256);
			player.message_time = GetTickCount();
		}
		else if (other_id < NPC_START) {
			wcsncpy_s(skelaton[other_id].message, my_packet->message, 256);
			skelaton[other_id].message_time = GetTickCount();
		}
		else {
			wcsncpy_s(npc[other_id - NPC_START].message, my_packet->message, 256);
			npc[other_id - NPC_START].message_time = GetTickCount();
		}
		break;

	}*/
	default:
		printf("Unknown PACKET type [%d]\n", ptr[1]);
	}
}

void ServerConnect::ReadPacket()
{
	DWORD iobyte, ioflag = 0;

	int ret = WSARecv(m_socket, &recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
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

void ServerConnect::SendPacket(int x, int y)
{
	cs_packet_up *my_packet = reinterpret_cast<cs_packet_up *>(send_buffer);
	my_packet->size = sizeof(my_packet);
	send_wsabuf.len = sizeof(my_packet);
	DWORD iobyte;
	if (0 != x) {
		if (1 == x) my_packet->type = CS_RIGHT;
		else my_packet->type = CS_LEFT;
		int ret = WSASend(m_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
		if (ret) {
			int error_code = WSAGetLastError();
			printf("Error while sending packet [%d]", error_code);
		}
	}
	if (0 != y) {
		if (1 == y) my_packet->type = CS_DOWN;
		else my_packet->type = CS_UP;
		WSASend(m_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	}
}

int ServerConnect::InputID()
{
	int id;
	cout << "ID¸¦ ÀÔ·Â : ";
	cin >> id;
	return id;
	
}
