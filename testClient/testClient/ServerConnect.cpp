#include "stdafx.h"
#include "ServerConnect.h"

//#define TEST

ServerConnect::ServerConnect() : m_InputKeyData(0)
{
	int retval = -1;
	retval = ConnectServer();
	if (retval == SOCKET_ERROR) {
		cout << "연결에 실패했습니다. IP주소와 포트번호를 확인해주세요." << endl;
		exit(1); //연결 실패시 프로그램 종료
	}
}


ServerConnect::~ServerConnect()
{
	closesocket(m_socket);
	WSACleanup();
}

int ServerConnect::ConnectServer()
{
	int retval;
	SOCKADDR_IN clientAddr;
	u_short port=0;
	char ipAddress[16];

	if (WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0)
		return 1;

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == INVALID_SOCKET)
		error_quit("socket()");

	memset(ipAddress, 0, sizeof(char) * 16);

#if defined TEST
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	clientAddr.sin_port = htons(9000);
#else
	std::cout << "통신할 IP주소 : ";
	std::cin.get(ipAddress, sizeof(ipAddress));
	//std::cout << IPbuf << std::endl;
	std::cout << "통신할 포트번호 : ";
	std::cin >> port;

	clientAddr.sin_family = AF_INET;
	clientAddr.sin_addr.s_addr = inet_addr(ipAddress);
	clientAddr.sin_port = htons(port);
#endif
	retval = connect(m_socket, (SOCKADDR*)&clientAddr, sizeof(clientAddr));

	return retval;
}

int ServerConnect::SendData()
{
	int retval;
	retval = send(m_socket, (char*)&m_InputKeyData, sizeof(WORD), 0);
	//if (retval == SOCKET_ERROR) {
	//	error_display("send()");
	//	return retval;
	//}

	m_InputKeyData = 0;
	return retval;
}

int ServerConnect::RecvData()
{
	int retval;
	retval = recvn(m_socket, m_recvData, sizeof(WORD)*2, 0);
	//if (retval == SOCKET_ERROR)
	//	error_display("recv()");
//#ifdef _DEBUG
//	int xPos = m_recvData[0];
//	int yPos = m_recvData[1];
//	cout << xPos<<"\t"<< yPos << endl;
//#endif
	return retval;
}
