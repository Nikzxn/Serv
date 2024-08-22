#include "Client.h"

Nikson::Client::Client()
{
}

Nikson::Client::~Client()
{
}

bool Nikson::Client::log_in(const std::string& nn, const std::string& pw)
{
	req_mtx.lock();
	ts << Token{Kind::log_in} << Token{Kind::data, nn} << Token{Kind::data, pw};
	req_mtx.unlock();
}

bool Nikson::Client::log_out()
{
	return false;
}

bool Nikson::Client::create_account(const std::string& nn, const std::string& pw)
{
	return false;
}

bool Nikson::Client::send_to(const std::string& nn, const std::string& dt)
{
	return false;
}

bool Nikson::Client::get_data()
{
	if (!(is_connected() && is_logged()))
		return false;

	req_mtx.lock();
	ts << Token{Kind::get_data};
	send();
	while (true)
	{
		recv();
		Token k = ts.get();
		switch (k.kind()) {
		case Kind::get_data:

		}
	}
	req_mtx.unlock();
	return true;
}



void Nikson::Client::send()
{
	Send(server, ts.data(), ts.size(), 0);
}

int Nikson::Client::recv()
{
	char buffer[CLIENT_BUFF] = {};//all zeros
	int size = Recv(server, buffer, sizeof buffer, 0);
	constexpr int fin_pack = 0;
	if (size == fin_pack)
		error();
	ts.load(buffer, size);
	return size;
}

bool Nikson::Client::wait_result()
{
	recv();
	Token k = ts.get();
	switch (k.kind()) {
	case Kind::good:
		return true;
	case Kind::bad:
		return false;
	default:
		winsock_error(no_one(), Where::somewhere);
	}
}

bool Nikson::Client::connect(const std::string& a,u_short port)
{
	if (server != not_connected())
		error();
	server = Socket(PF_INET, SOCK_STREAM, 0);
	sockaddr_in sai = {};
	sai.sin_family = AF_INET;
	if (inet_pton(AF_INET, a.c_str(), &sai.sin_addr) < 0)
		error();
	sai.sin_port = htons(port);
	Connect(server, (SA*)&sai, 0);
	return wait_result();
}

void Nikson::Client::disconnect()
{
	if (server == not_connected())
		error();
	Close(server);
	server = not_connected();
	logged = false;
}
