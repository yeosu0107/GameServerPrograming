#include <thread>
#include <iostream>

using namespace std;

volatile int g_data;
volatile bool flag = false;

void recv_thread() {
	while (flag == false);
	cout << "I got [ " << g_data << " ]" << endl;
}

void send_thread() {
	g_data = 999;
	cout << "Flag is True" << endl;
	flag = true;
}


int main(void)
{
	thread r_th{ recv_thread };
	thread s_th{ send_thread };
	s_th.join();
	r_th.join();
	
	system("pause");
}