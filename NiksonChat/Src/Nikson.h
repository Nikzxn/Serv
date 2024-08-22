#pragma once
#pragma comment(lib, "ws2_32.lib")
#include <string>
#include <WS2tcpip.h>
#define SA sockaddr
#define BUFF_SIZE 1024



namespace Nikson 
{
	inline int winsock_last_error() { return WSAGetLastError(); }//winsock_last_error
	constexpr SOCKET no_one() {
		return INVALID_SOCKET;
	}
	inline const std::string& loopback() { static std::string lb = "127.0.0.1"; return lb; }
	enum class Where {
		somewhere=-1,socket, accept, bind, connect,listen,
		send, recv, close, select, FD_SET,
		WSAStratup
	};
	class Winsock_error
	{
	public:
		
		Winsock_error(SOCKET wh, Where whr,int cd, const std::string& msg);
		Winsock_error(SOCKET wh, Where whr, int cd);
		int		code() const {return error_code;}
		SOCKET	who() const { return sock; }
		const std::string&	what() const { return message; }
		Where				where() const { return func; }
		
	private:
		int error_code;
		std::string message;
		Where func;
		SOCKET sock;
	};//Winsock_error
	inline void winsock_error(SOCKET wh, Where whr,
		const std::string& wht) {
		throw Winsock_error{wh,whr,winsock_last_error(),wht};
	}//winsock_error
	inline void winsock_error(SOCKET wh, Where whr) {
		throw Winsock_error{wh, whr, winsock_last_error()};
	}//winsock_error

	class Winsock 
	{
	public:
		Winsock();
		~Winsock();
	private:
		WSADATA wsaData;
	};//Winsock
	
	SOCKET make_server(int af, u_short port);

	/*////////////////////////////////////////////////////////////
	* ������� ��������. 
	* ���������� ���� Winsock_error, ���� Socket_error ����������.
	* 
	* ����������: ������� �������� ����� ��� ���������� �������,
	* �� � ��������� ����� (socket->Socket)
	*/////////////////////////////////////////////////////////////

	//����������: ������������� ����� - ���������� �������������� ������
	SOCKET Socket(int af, int type, int protocol);
	//����������: ������������� ����� - ���������� ������������ ������
	SOCKET Accept(SOCKET   s, sockaddr* addr, int* addrlen);
	void Listen(SOCKET s, int    backlog);
	void Bind(SOCKET s, const sockaddr* addr, int namelen);
	void Connect(SOCKET s, const sockaddr* name, int namelen);
	void Close(SOCKET sockfd);
	//����������: ������������� ����� - ���-�� ������������ ������
	// 0 (����� �����), ���� ����� �� ��������������� ����� ���������� ����������� (���� ������� ������� FIN)
	int Recv(SOCKET sockfd, char* st, int nbytes, int flags);
	//����������: ������������� ����� - ���-�� ���������� ������
	int Send(SOCKET sockfd, const char* st, int nbytes, int flags);
	//����������: ������������� ����� - ������� ������� ������������,
	//0 � ������ ���� - ����
	int Select(int nfds,fd_set* readfds,fd_set* writefds,fd_set* exceptfds,const timeval* timeout);
	void Fd_set(SOCKET fd, fd_set* set);
	

}//Nikson