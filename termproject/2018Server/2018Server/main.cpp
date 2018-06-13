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

		// 에러 처리
		if (is_success == FALSE) {
			//cout << "Error is GQCS key [" << key << "]\n";
			Server::getInstance()->DisconnectPlayer(key);
			continue;
		}
		// 접속 종료 처리
		if (data_size == 0 && key < NPC_START) {
			Server::getInstance()->DisconnectPlayer(key);
			continue;
		}

		// Send / Recv 처리
		EXOver* exover = reinterpret_cast<EXOver*>(p_over);
		if (exover->event_type == EV_RECV) {
			Server::getInstance()->recv(key, data_size, exover);
		}
		else if (exover->event_type == EV_MOVE) {
			Server::getInstance()->MoveNpc((int)key);
			delete exover;
		}
		else if(exover->event_type==EV_SEND) {
			delete exover;
		}
		else if (exover->event_type == EV_DBUPDATE) {
			Server::getInstance()->UploadUserDatatoDB();
			delete exover;
		}
		else if (exover->event_type == EV_PLAYER_MOVE) {
			Server::getInstance()->NPC_AI((int)key, (int)exover->event_target);
			delete exover;
		}
		else if (exover->event_type == EV_MOVE_DIR) {
			Server::getInstance()->MoveDirNpc((int)key, (int)exover->event_target);
			delete exover;
		}
		else {
			//cout << "Unknown Event Error in Worker Thread\n";
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
	priority_queue<Event*, vector<Event*>, compare>* event_queue 
		= Server::getInstance()->getEventQueue();

	while (true) {
		if (event_queue->empty())
			continue;
		Server::getInstance()->getMutex()->lock();
		Event* nowEvent = event_queue->top();
		event_queue->pop();
		Server::getInstance()->getMutex()->unlock();
		chrono::duration<double> duration = chrono::system_clock::now() - nowEvent->startClock;
		if (duration.count() > nowEvent->time) {
			if (nowEvent->type == MOVE_AROUND_TYPE) {
				EXOver* over = new EXOver();
				over->event_type = EV_MOVE;

				PostQueuedCompletionStatus(*Server::getInstance()->getIOCP(),
					0, nowEvent->id, &over->wsaover);
			}
			else if(nowEvent->type == DB_UPDATE_TYPE) {
				EXOver* over = new EXOver();
				over->event_type = EV_DBUPDATE;

				PostQueuedCompletionStatus(*Server::getInstance()->getIOCP(),
					0, nowEvent->id, &over->wsaover);
			}
			else if (nowEvent->type == MOVE_DIR_TYPE) {
				EXOver* over = new EXOver();
				over->event_type = EV_MOVE_DIR;
				over->event_target = nowEvent->target;
				over->event_info = nowEvent->info;

				PostQueuedCompletionStatus(*Server::getInstance()->getIOCP(),
					0, nowEvent->id, &over->wsaover);
			}
			delete nowEvent;
		}
		else {
			//아직 시간이 아니면 현재시간으로 바꾸고, 남은 시간 조절하고 다시 푸시
			nowEvent->startClock = chrono::system_clock::now();
			nowEvent->time -= duration.count();
			Server::getInstance()->getMutex()->lock();
			//event_queue->push(nowEvent);
			event_queue->emplace(nowEvent);
			Server::getInstance()->getMutex()->unlock();
		}
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
				nowEvent->client->player_id = nowEvent->id;
				Server::getInstance()->AcceptNewClient(nowEvent->client, nowEvent->clientIndex);
			}
			else {
				//cout << "Unknown ID" << endl;
			}	
		}
		db_queue->pop();
	}
}

int main(void)
{	
	vector<thread> all_threads;
	Server::getInstance()->Initialize();
#ifdef DB
	cout << "set DB connect : true" << endl;
#else
	cout << "set DB Connect : false" << endl;
#endif
	for (int i = 0; i < 4; ++i) {
		all_threads.push_back(thread(WorkerThread));
	}
	all_threads.push_back(thread(AcceptThread));
	all_threads.push_back(thread(TimerThread));
#ifdef DB
	all_threads.push_back(thread(DBThread));
#endif
	for (auto& th : all_threads) {
		th.join();
	}
}