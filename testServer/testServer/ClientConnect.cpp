#include "stdafx.h"
#include "ClientConnect.h"


ClientConnect::ClientConnect(SOCKET tmpSocket) :
	m_socket(tmpSocket), move(false), m_playerNum(0)
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

int ClientConnect::sendData()
{
	int retval;
	memset(m_sendBuf, 0, sizeof(char)*MAX_BUF); //버퍼 초기화
	char clientnum[1] = { g_nClient };
	if (move) {
		PlayerInfo* tt = new PlayerInfo(m_playerNum, xPos, yPos);
		memcpy(m_sendBuf, tt, sizeof(tt));
		m_sendLen[0] = sizeof(PlayerInfo);
		
		m_sendBuf[0] = xPos;
		m_sendBuf[1] = yPos;
	}
	else {
		m_sendBuf[0] = -1;
		m_sendLen[0] = 1;
	}

	retval = send(m_socket, clientnum, 1, 0);		//클라이언트 갯수
	retval = send(m_socket, m_sendLen, 1, 0);	//받는 데이터 크기
	retval = send(m_socket, m_sendBuf, (int)m_sendLen[0], 0); //데이터
	return retval;
}

void ClientConnect::MoveObject()
{
	move = false;
	if (*m_recvBuf & DIR_DOWN) {
		if (yPos < MAX_Y)
			yPos += 1;
		//cout << "down" << endl;
		move = true;
	}
	if (*m_recvBuf & DIR_UP) {
		if (yPos > 0)
			yPos -= 1;
		//cout << "up" << endl;
		move = true;
	}
	if (*m_recvBuf & DIR_LEFT) {
		if (xPos > 0)
			xPos -= 1;
		//cout << "left" << endl;
		move = true;
	}
	if (*m_recvBuf & DIR_RIGHT) {
		if (xPos < MAX_X)
			xPos += 1;
		//cout << "right" << endl;
		move = true;
	}
//#ifdef _DEBUG
//	if (move) {
//		cout << xPos << "\t" << yPos << endl;
//	}
//#endif
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
	if (g_nClient >= 10) {
		cout << "클라이언트 Limited" << endl;
		return false;
	}
	ClientConnect* client = new ClientConnect(socket);
	client->setPlayer(g_nClient);
	g_ClientArray[g_nClient] = client;
	g_nClient += 1;

	return true;
}

void RemoveSocket(int index)
{
	ClientConnect* client = g_ClientArray[index];

	closesocket(client->getSocket());
	delete client;

	for (int i = index; i < g_nClient; ++i)
		g_ClientArray[i] = g_ClientArray[i + 1];
	
	if(g_ClientArray[g_nClient-1])
		delete g_ClientArray[g_nClient - 1];

	g_nClient -= 1;
}
