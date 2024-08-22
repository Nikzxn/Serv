#include "Client.h"
#include <iostream>
using namespace std;
void cb(Nikson::Client::Error_kind ek) { std::cerr << "error!" << '\n'; }
int main() {
	Nikson::Winsock ws;
	Nikson::Client client;
	client.set_callback(&cb);
	int sec = 0;
	while (!client.connect(Nikson::loopback(), 13))
	{
		Sleep(1000);
		++sec;
		cout << sec << '\n';
	}
	
	string n, p;
	cin >> n >> p;
	client.create_account(n, p);
	bool res=client.log_in(n, p);
	client.start_get_data();
	cin.clear();
	char c; cin >> c;
	return 0;
}