// testServer.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
//#include "ClientConnect.h"



int main()
{
	int retval;

	bool serverLoop = true;


	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET)
		err_display("socket()");

	//bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) 
		err_display("bind()");

	cout << "클라이언트 접속 대기중" << endl;

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) 
		err_display("listen()");

	//넌블로킹 소켓
	unsigned long on = true;
	retval = ioctlsocket(listen_sock, FIONBIO, &on);
	if (retval == SOCKET_ERROR)
		err_display("ioctlsocket()");

	FD_SET frecv;
	FD_SET fsend;
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen= sizeof(clientaddr);

	while (serverLoop) {
		FD_ZERO(&frecv);
		FD_ZERO(&fsend);

		FD_SET(listen_sock, &frecv);

		for (int i = 0; i < g_nClient; ++i) {
			if (g_ClientArray[i]->getType() == PacketType::sendType)
				FD_SET(g_ClientArray[i]->getSocket(), &fsend);
			else
				FD_SET(g_ClientArray[i]->getSocket(), &frecv);
		}

		retval = select(0, &frecv, &fsend, NULL, NULL);
		//cout << WSAGetLastError() << endl;
		if (retval == SOCKET_ERROR)
			err_display("select()");

		//#1 - 클라이언트 접속
		if (FD_ISSET(listen_sock, &frecv)) {
			client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
			if (client_sock == INVALID_SOCKET) {
				err_display("accept()");
			}

			printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
				inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

			if (AddSocket(client_sock) == false) {
				cout << "소켓 생성 에러" << endl;
				closesocket(client_sock);
			}
		}

		//#2 데이터 통신
		for (int i = 0; i < g_nClient; ++i) {
			ClientConnect* client = g_ClientArray[i];

			//recv
			if (FD_ISSET(client->getSocket(), &frecv)) {
				retval = client->recvData();
				if (retval == SOCKET_ERROR) {
					if (retval != 0) {
						err_display("recv()");
					}
					RemoveSocket(i);
					if (g_nClient == 0) {
						serverLoop = false;
						break;
					}
					continue;
				}
				//recv 성공시 말 움직이기
				client->MoveObject();
				client->setType(PacketType::sendType);
			}

			//send
			if (FD_ISSET(client->getSocket(), &fsend)) {
				retval = client->sendData();
				if (retval == SOCKET_ERROR) {
					if (retval != 0) {
						err_display("recv()");
					}
					RemoveSocket(i);
					if (g_nClient == 0) {
						serverLoop = false;
						break;
					}
					continue;
				}
				client->setType(PacketType::recvType);
			}
		}
	}
	//SOCKADDR_IN clientaddr;
	//int addrlen= sizeof(clientaddr);;
	//SOCKET* client_sock=new SOCKET();

	//*client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);

	//if (*client_sock == INVALID_SOCKET) {
	//	err_display("accept()");
	//}
	//// 접속한 클라이언트 정보 출력
	//printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
	//	inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	//
	//ClientConnect* client = new ClientConnect(client_sock);
	//while (1) {
	//	retval = client->sendData();
	//	if (retval == SOCKET_ERROR)
	//		break;
	//	retval = client->recvData();
	//	if (retval == SOCKET_ERROR)
	//		break;
	//	client->MoveObject();
	//}

	cout << "모든 클라이언트 연결이 끊어졌습니다" << endl;
	cout << "서버를 종료합니다" << endl;

	//delete client;
	closesocket(listen_sock);
	WSACleanup();
    return 0;
}