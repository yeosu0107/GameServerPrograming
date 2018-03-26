#include "stdafx.h"
#include "ClientConnect.h"
#include <deque>
//UINT g_nextID = 0;
deque<UINT> g_nextID = { 0,1,2,3,4,5,6,7,8,9 };

ClientConnect::ClientConnect(SOCKET tmpSocket) :
	m_socket(tmpSocket),  m_playerNum(0)
{
	memset(m_recvBuf, 0, sizeof(char)*MAX_BUF);
	memset(m_sendBuf, 0, sizeof(char)*MAX_BUF);

	xPos = 0;
	yPos = 0;

	m_packet = PacketType::sendType;
	//cout << xPos << "\t" << yPos << endl;
}


ClientConnect::~ClientConnect()
{
	closesocket(m_socket);
}

int ClientConnect::recvData()
{
	int retval;
	retval = recvn(m_socket, m_recvBuf, sizeof(WORD), 0);
	//if (retval == SOCKET_ERROR)
	//	err_display("recv()");
	return retval;
}

int ClientConnect::sendData(int index)
{
	int retval;
	memset(m_sendBuf, 0, sizeof(char)*MAX_BUF); //버퍼 초기화

	g_Player[index].set(m_playerNum, xPos, yPos);

	memcpy(m_sendBuf, g_Player, sizeof(PlayerInfo)*g_nClient);
	m_sendLen[0] = g_nClient;
	m_sendLen[1] = sizeof(PlayerInfo)*g_nClient;

	//PlayerInfo* tt = new PlayerInfo();
	//tt->set(0, xPos, yPos);
	//tt->print();
	//memcpy(m_sendBuf, tt, sizeof(PlayerInfo));
	//m_sendLen[0] = sizeof(PlayerInfo);

	//retval = send(m_socket, clientnum, 1, 0);		//클라이언트 갯수
	retval = send(m_socket, m_sendLen, 2, 0);	// 클라이언트 갯수 & 받는 데이터 크기
	retval = send(m_socket, m_sendBuf, (int)m_sendLen[1], 0); //데이터
	return retval;
}

void ClientConnect::MoveObject()
{
	if (*m_recvBuf & DIR_DOWN) {
		if (yPos < MAX_Y)
			yPos += 1;
	}
	if (*m_recvBuf & DIR_UP) {
		if (yPos > 0)
			yPos -= 1;
	}
	if (*m_recvBuf & DIR_LEFT) {
		if (xPos > 0)
			xPos -= 1;
	}
	if (*m_recvBuf & DIR_RIGHT) {
		if (xPos < MAX_X)
			xPos += 1;
	}
}

void ClientConnect::setType(PacketType tmp)
{
	m_packet = tmp;
}

void ClientConnect::setPlayer(UINT tmp)
{
	m_playerNum = tmp;
}

bool AddSocket(SOCKET socket)
{
	if (g_nClient >= MAX_CLIENT) {
		cout << "클라이언트 Limited" << endl;
		return false;
	}
	ClientConnect* client = new ClientConnect(socket);
	client->setPlayer(g_nextID[0]);
	g_nextID.pop_front();
	//if (g_nextID >= MAX_CLIENT)
	//	g_nextID = 0;
	for (int i = 0; i < MAX_CLIENT; ++i) {
		if (g_ClientArray[i] == nullptr) {
			//client->setPlayer(i);
			g_ClientArray[i] = client;
			break;
		}
	}
	
	g_nClient += 1;

	return true;
}

void RemoveSocket(int index)
{
	ClientConnect* client = g_ClientArray[index];
	g_nextID.push_back(g_ClientArray[index]->getPlayerNum());
	closesocket(client->getSocket());
	delete client;

	for (int i = index; i < g_nClient; ++i) {
		g_ClientArray[i] = g_ClientArray[i + 1];
		g_Player[i] = g_Player[i + 1];
	}
	
	if(g_ClientArray[g_nClient-1])
		delete g_ClientArray[g_nClient - 1];
	g_ClientArray[g_nClient - 1] = nullptr;

	g_nClient -= 1;
}
