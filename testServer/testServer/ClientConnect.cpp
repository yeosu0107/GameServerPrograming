#include "stdafx.h"
#include "ClientConnect.h"


ClientConnect::ClientConnect(SOCKET* tmpSocket) :
	m_socket(tmpSocket), move(false)
{
	memset(m_recvBuf, 0, sizeof(char)*MAX_BUF);
	memset(m_sendBuf, 0, sizeof(char)*MAX_BUF);

	xPos = 0;
	yPos = 0;
	cout << xPos << "\t" << yPos << endl;
}


ClientConnect::~ClientConnect()
{
	closesocket(*m_socket);
}

int ClientConnect::recvData()
{
	int retval;
	retval = recvn(*m_socket, m_recvBuf, sizeof(WORD), 0);
	if (retval == SOCKET_ERROR)
		err_display("recv()");
	return retval;
}

int ClientConnect::sendData()
{
	int retval;
	memset(m_sendBuf, 0, sizeof(char)*MAX_BUF); //버퍼 초기화
	
	if (move) {
		m_sendBuf[0] = xPos;
		m_sendBuf[1] = yPos;
	}
	else {
		m_sendBuf[0] = -1;
	}
	
	retval = send(*m_socket, m_sendBuf, sizeof(SendDataFormat), 0);
	return 0;
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
