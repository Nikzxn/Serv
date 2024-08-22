#include "Nikson.h"


Nikson::Winsock::Winsock()
{
	int result;

	// Initialize Winsock
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	constexpr int success = 0;
	if (result != success) {

		winsock_error(no_one(), Where::WSAStratup);
	}
}

Nikson::Winsock::~Winsock()
{
	WSACleanup();
}

SOCKET Nikson::make_server(int af, u_short port)
{
	
	switch (af) {
	case AF_INET:
	{
		SOCKET server = Socket(AF_INET, SOCK_STREAM, 0);
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(port);
		Bind(server, (SA*)&addr, sizeof addr);
		Listen(server, SOMAXCONN);
		return server;

	}
	case AF_INET6:
		winsock_error(no_one(), Where::somewhere, "AF_INET6 is not supported yet");
	}
}

SOCKET Nikson::Socket(int af, int type, int protocol)
{
	SOCKET sock = socket(af, type, protocol);
	if (sock == INVALID_SOCKET)
		winsock_error(no_one(),Where::socket);
	return sock;
}

void Nikson::Listen(SOCKET s, int backlog)
{
	if (listen(s, backlog) == SOCKET_ERROR)
		winsock_error(s, Where::listen);
}

void Nikson::Bind(SOCKET s, const sockaddr* addr, int namelen)
{
	if (bind(s, addr, namelen) == SOCKET_ERROR)
		winsock_error(s, Where::bind);
		
}

SOCKET Nikson::Accept(SOCKET s, sockaddr* addr, int* addrlen)
{
	SOCKET sock = accept(s, addr, addrlen);
	if (sock == INVALID_SOCKET)
		winsock_error(s, Where::accept);
	return sock;
}

void Nikson::Connect(SOCKET s, const sockaddr* name, int namelen)
{
	if (connect(s, name, namelen) == SOCKET_ERROR)
		winsock_error(s, Where::connect);
}

void Nikson::Close(SOCKET sockfd)
{
	if (closesocket(sockfd) == SOCKET_ERROR)
		winsock_error(sockfd, Where::close);
}

int Nikson::Recv(SOCKET sockfd, char* st, int nbytes, int flags)
{
	int size = recv(sockfd, st, nbytes, flags);
	if (size == SOCKET_ERROR)
		winsock_error(sockfd, Where::recv);
	return size;
}

int Nikson::Send(SOCKET sockfd, const char* st, int nbytes, int flags)
{
	int size = send(sockfd, st, nbytes, flags);
	if (size == SOCKET_ERROR)
		winsock_error(sockfd, Where::send);
	return size;
}

int Nikson::Select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, const timeval* timeout)
{
	int count = select(nfds, readfds, writefds, exceptfds, timeout);
	if (count == SOCKET_ERROR)
		winsock_error(no_one(), Where::select);
	return count;
}

void Nikson::Fd_set(SOCKET fd, fd_set* set)
{
	if (set->fd_count == FD_SETSIZE)
		winsock_error(no_one(),Where::FD_SET,"fd_set is full");
	FD_SET(fd, set);
}





Nikson::Winsock_error::Winsock_error(SOCKET wh, Where whr, int cd, const std::string& msg)
	:sock{wh}, func{whr}, error_code{cd}, message{msg} {}

Nikson::Winsock_error::Winsock_error(SOCKET wh, Where whr, int cd)
	:sock{ wh }, func{ whr }, error_code{ cd } {}

