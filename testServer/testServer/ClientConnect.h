#pragma once

const int MAX_X = 7;
const int MAX_Y = 7;

#define MAX_BUF 512
#define MAX_CLIENT 10

enum PacketType {
	sendType = 0, recvType = 1
};

#pragma pack(1)
struct PlayerInfo {
	int	m_playerNum;
	int xPos;
	int yPos;

	void print() {
		printf("%d	%d	%d\n", m_playerNum, xPos, yPos);
	}
	void set(int num, int x, int y) {
		m_playerNum = num;
		xPos = x;
		yPos = y;
	}
};
#pragma pack()

class ClientConnect
{
private:
	SOCKET						m_socket;
	char							m_recvBuf[MAX_BUF];
	char							m_sendBuf[MAX_BUF];
	char							m_sendLen[2];

	UINT							m_playerNum;
	int								xPos, yPos;

	PacketType				m_packet;
public:
	ClientConnect(SOCKET tmpSocket);
	~ClientConnect();

	int recvData();
	int sendData(int index);

	void setType(PacketType tmp);
	void setPlayer(UINT tmp);
	void MoveObject();

	SOCKET getSocket() { return m_socket; }
	PacketType getType() { return m_packet; }
	UINT	getPlayerNum() const { return m_playerNum; }
};

bool AddSocket(SOCKET socket);
void RemoveSocket(int index);