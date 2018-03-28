#include <thread>
#include <iostream>
#include <chrono>
#include <vector>
#include <mutex>

using namespace std;
using namespace std::chrono;


volatile int sum;
mutex mylock;

void thread_func(int num_thread, int th_id)
{
	volatile int local_sum = 0;
	for (int i = 0; i < 50000000 / num_thread; ++i) {
		local_sum += 2;
	}
	mylock.lock();
	sum += local_sum;
	mylock.unlock();
}

int main()
{
	sum = 0;
	auto t = high_resolution_clock::now();
	for (int i = 0; i < 50000000; ++i) sum += 2;
	auto d = high_resolution_clock::now() - t;
	cout << "Single Thread Time : " << duration_cast<milliseconds>(d).count() <<" (millisec)"<< endl;
	cout << "Result : " << sum << endl<<endl;

	for (int num_thread = 2; num_thread <= 16; num_thread*=2) {
		sum = 0;
		vector<thread*> my_threads;
		t = high_resolution_clock::now();
		for (int i = 0; i < num_thread; ++i) {
			my_threads.push_back(new thread{ thread_func, num_thread, i });
		}

		for (auto th : my_threads) {
			th->join();
			delete th;
		}

		d = high_resolution_clock::now() - t;
		cout << num_thread << " Thread Time : " << duration_cast<milliseconds>(d).count() << " (millisec)" << endl;
		cout << "Result : " << sum << endl << endl;
		my_threads.clear();
	}
	system("Pause");
}