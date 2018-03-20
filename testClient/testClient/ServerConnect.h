#pragma once


class ServerConnect
{
private:
	WSADATA	m_wsa;
	SOCKET		m_socket;
	WORD		m_InputKeyData;
	char			m_recvData[MAX_BUF];
	char			m_recvLen[1];
	
	//RecvDataFormat		m_recvData;
public:
	ServerConnect();
	~ServerConnect();
	int ConnectServer();
	int SendData();
	int RecvData();
	WORD& getKeyData() { return m_InputKeyData; }
	char*		getRecvData() { return m_recvData; }
};

