#include "stdafx.h"
#include "Server.h"
#include "DBConnect.h"
#include <thread>
#include <queue>

DBConnect DBprocess;

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
		if (data_size == 0 && key < NPC_START) {
			Server::getInstance()->DisconnectPlayer(key);
			continue;
		}

		// Send / Recv 贸府
		EXOver* exover = reinterpret_cast<EXOver*>(p_over);
		if (exover->event_type == EV_RECV) {
			Server::getInstance()->recv(key, data_size, exover);
		}
		else if (exover->event_type == EV_MOVE) {
			Server::getInstance()->MoveNpc((int)key);
		}
		else if(exover->event_type==EV_SEND) {
			delete exover;
		}
		else {
			cout << "Unknown Event Error in Worker Thread\n";
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
		Server::getInstance()->AcceptAndSearchClient(g_socket);
	}
}

void TimerThread() {
	auto event_queue = Server::getInstance()->getEventQueue();
	while (true) {
		if (event_queue->empty())
			continue;
		Server::getInstance()->getMutex()->lock();
		Event* nowEvent = event_queue->top();
		chrono::duration<double> duration = chrono::system_clock::now() - nowEvent->startClock;
		if (duration.count() > nowEvent->time) {
			if (nowEvent->type == MOVE_TYPE) {
				
				event_queue->pop();
				Server::getInstance()->getMutex()->unlock();

				EXOver* over = new EXOver();
				over->event_type = EV_MOVE;

				PostQueuedCompletionStatus(*Server::getInstance()->getIOCP(),
					0, nowEvent->id, &over->wsaover);
				//delete nowEvent;
				
			}
		}
		else
			Server::getInstance()->getMutex()->unlock();
	}
}

void DBThread() {
	auto db_queue = Server::getInstance()->getDB();
	while (true) {
		if (db_queue->empty())
			continue;
		DBEvent* nowEvent = &db_queue->front();
		if (nowEvent->type == UPDATE_POS) {
			DBprocess.UpdateUserPos(nowEvent->id, nowEvent->client->x, nowEvent->client->y);
		}
		else if (nowEvent->type == SEARCH_ID) {
			bool ret = DBprocess.SearchUserAndLogin(nowEvent->id, nowEvent->client->x, nowEvent->client->y);
			if (ret) {
				Server::getInstance()->AcceptNewClient(nowEvent->client, nowEvent->clientIndex);
			}
			else {
				cout << "Unknown ID" << endl;
			}	
		}
		db_queue->pop();
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
	all_threads.push_back(thread(DBThread));
	for (auto& th : all_threads) {
		th.join();
	}
}