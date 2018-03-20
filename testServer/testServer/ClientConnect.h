#pragma once

const int MAX_CLIENT = 10;
const int MAX_X = 7;
const int MAX_Y = 7;

#define MAX_BUF 512

enum PacketType {
	sendType = 0, recvType = 1
};

struct PlayerInfo {
	UINT	m_playerNum;
	int xPos, yPos;

	PlayerInfo(UINT num, int x, int y) {
		m_playerNum = num;
		xPos = x;
		yPos = y;
	}
};

class ClientConnect
{
private:
	SOCKET						m_socket;
	char							m_recvBuf[MAX_BUF];
	char							m_sendBuf[MAX_BUF];
	char							m_sendLen[1];

	bool							move;
	UINT							m_playerNum;
	int								xPos, yPos;

	PacketType				m_packet;
public:
	ClientConnect(SOCKET tmpSocket);
	~ClientConnect();

	int recvData();
	int sendData();

	void setType(PacketType tmp);
	void setPlayer(UINT tmp);
	void MoveObject();

	SOCKET getSocket() { return m_socket; }
	PacketType getType() { return m_packet; }
	
};

//extern ClientConnect* g_ClientArray[MAX_CLIENT];
//extern UINT g_nClient;

bool AddSocket(SOCKET socket);
void RemoveSocket(int index);