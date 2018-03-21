#include "stdafx.h"
#include "ClientConnect.h"


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

int ClientConnect::sendData()
{
	int retval;
	memset(m_sendBuf, 0, sizeof(char)*MAX_BUF); //���� �ʱ�ȭ
	char clientnum[1] = { g_nClient };

	g_Player[m_playerNum].set(m_playerNum, xPos, yPos);

	memcpy(m_sendBuf, g_Player, sizeof(PlayerInfo)*g_nClient);
	m_sendLen[0] = sizeof(PlayerInfo)*g_nClient;

	//PlayerInfo* tt = new PlayerInfo();
	//tt->set(0, xPos, yPos);
	//tt->print();
	//memcpy(m_sendBuf, tt, sizeof(PlayerInfo));
	//m_sendLen[0] = sizeof(PlayerInfo);

	retval = send(m_socket, clientnum, 1, 0);		//Ŭ���̾�Ʈ ����
	retval = send(m_socket, m_sendLen, 1, 0);	//�޴� ������ ũ��
	retval = send(m_socket, m_sendBuf, (int)m_sendLen[0], 0); //������
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
	if (g_nClient >= 10) {
		cout << "Ŭ���̾�Ʈ Limited" << endl;
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
