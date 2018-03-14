#pragma once

const int MAX_X = 7;
const int MAX_Y = 7;

class ClientConnect
{
private:
	SOCKET	*					m_socket;
	char							m_recvBuf[MAX_BUF];
	char							m_sendBuf[MAX_BUF];

	bool							move;
	int								xPos, yPos;
public:
	ClientConnect(SOCKET* tmpSocket);
	~ClientConnect();

	int recvData();
	int sendData();

	void MoveObject();
};

