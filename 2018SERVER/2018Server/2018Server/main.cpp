#include <thread>

#include "stdafx.h"
#include "Server.h"

void WorkerThread()
{
	while (true) {
		unsigned long			data_size;
		unsigned long long	key;
		WSAOVERLAPPED*	p_over=nullptr;

		//BOOL is_success = Server::getInstance()->GCCS(data_size, key, p_over);
		BOOL is_success = GetQueuedCompletionStatus(*Server::getInstance()->getIOCP(),
			&data_size, &key, &p_over, INFINITE);
		cout << "GQCS from client [" << key << "] with size [" << data_size << "]\n";

		// 俊矾 贸府
		if (is_success == FALSE) {
			cout << "Error is GQCS key [" << key << "]\n";
			Server::getInstance()->DisconnectPlayer(key);
			continue;
		}
		// 立加 辆丰 贸府
		if (data_size == 0) {
			Server::getInstance()->DisconnectPlayer(key);
			continue;
		}
		//Client* nowClient = Server::getInstance()->getClient(key)
		// Send / Recv 贸府
		EXOver* exover = reinterpret_cast<EXOver*>(p_over);
		if (exover->is_recv == true) {
			Server::getInstance()->recv(key, data_size, exover);
		}
		else {
			delete exover;
		}
	}
}

void AcceptThread()
{
	auto g_socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN bind_addr;
	ZeroMemory(&bind_addr, sizeof(SOCKADDR_IN));
	bind_addr.sin_family = AF_INET;
	bind_addr.sin_port = htons(MY_SERVER_PORT);
	bind_addr.sin_addr.s_addr = INADDR_ANY;

	::bind(g_socket, reinterpret_cast<sockaddr*>(&bind_addr),
		sizeof(SOCKADDR_IN));

	listen(g_socket, 1000);

	while (true) {
		Server::getInstance()->AcceptNewClient(g_socket);
	}
}

int main(void)
{	
	vector<thread> all_threads;
	Server::getInstance()->Initialize();
	for (int i = 0; i < 4; ++i) {
		all_threads.push_back(thread(WorkerThread));
	}
	all_threads.push_back(thread(AcceptThread));

	for (auto& th : all_threads) {
		th.join();
	}
}