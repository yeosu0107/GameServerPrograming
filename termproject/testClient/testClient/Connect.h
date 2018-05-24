#pragma once

#define	BUF_SIZE				1024

class ServerConnect {
private:
	WSADATA m_wsa;
	SOCKET		m_socket;
	int		g_myid;

	WSABUF	send_wsabuf;
	char 	send_buffer[BUF_SIZE];
	WSABUF	recv_wsabuf;
	char	recv_buffer[BUF_SIZE];
	char	packet_buffer[BUF_SIZE];

	DWORD		in_packet_size = 0;
	int		saved_packet_size = 0;
	
	
public:
	ServerConnect();
	~ServerConnect();

	void ProcessPacket(char* ptr);
	void ReadPacket();
	void SendPacket(int x, int y);
	int InputID();
};