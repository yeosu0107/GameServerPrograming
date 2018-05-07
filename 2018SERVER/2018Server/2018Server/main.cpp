#include "stdafx.h"
#include "Server.h"
#include <thread>
#include <queue>

void WorkerThread()
{
	while (true) {
		unsigned long			data_size;
		unsigned long long	key;
		WSAOVERLAPPED*	p_over=nullptr;

		BOOL is_success = GetQueuedCompletionStatus(*Server::getInstance()->getIOCP(),
			&data_size, &key, &p_over, INFINITE);
		//cout << "GQCS from client [" << key << "] with size [" << data_size << "]\n";

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

void TimerThread() {
	auto event_queue = Server::getInstance()->getEventQueue();
	while (true) {
		if (event_queue->empty())
			continue;
		Event nowEvent = event_queue->top();
		chrono::duration<double> duration = chrono::system_clock::now() - nowEvent.startClock;
		if (duration.count() > nowEvent.time) {
			if (nowEvent.type == MOVE_TYPE) {
				cs_packet_up move_packet;

				move_packet.type = rand() % 4 + 1;;
				move_packet.size = sizeof(move_packet);

				EXOver* over = new EXOver();
				over->is_recv = true;
				over->io_buf[0] = move_packet.size;
				over->io_buf[1] = move_packet.type;

				PostQueuedCompletionStatus(*Server::getInstance()->getIOCP(),
					over->io_buf[0], nowEvent.id, &over->wsaover);
				Server::getInstance()->getMutex()->lock();
				event_queue->pop();
				Server::getInstance()->getMutex()->unlock();
			}
			else if (nowEvent.type == RESERVE_TYPE) {
				Server::getInstance()->MoveNpc();

				Server::getInstance()->getMutex()->lock();
				event_queue->pop();
				Server::getInstance()->getMutex()->unlock();
			}
		}
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
	all_threads.push_back(thread(TimerThread));

	for (auto& th : all_threads) {
		th.join();
	}
}