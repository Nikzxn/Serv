#pragma once
#include "Nikson.h"
#include "Tokens.h"
#include <thread>
#include <mutex>
#include <map>
#ifndef CLIENT_BUFF
	#define CLIENT_BUFF BUFF_SIZE
#endif
namespace Nikson {
	inline SOCKET not_connected() { return INVALID_SOCKET; }
	inline char me() { return 'm'; }
	inline char he() { return 'h'; }
	inline char none() {return -1;}
	class Message 
	{
	public:
		Message(char who, data_t dt);
		char who(){ return wh; };
		const std::string& source() { return src; }
	private:
		char wh{none()};//who
		std::string src;
	};

	class Client {
	public:
		Client();
		~Client();
		bool log_in(const std::string& nn, const std::string& pw);
		bool log_out();
		bool create_account(const std::string& nn, const std::string& pw);
		bool send_to(const std::string& nn, const std::string& dt);
		bool get_data();
		void set_callback(void(c)(void)) { callback = c; };
		void set_current_char(const std::string& nn){init_vector(nn,) }
	private:
		void send();
		int recv();
		class Error {};
		void error() { throw Error{}; }
	private:
		bool wait_result();
		bool connect(const std::string& a, u_short port);
		void disconnect();
		bool is_logged() { return logged; }
		bool is_connected() { return server != not_connected(); }
		bool is_callback_on() { return callback != nullptr; }
	private:
		SOCKET server{ not_connected() };
		bool logged{ false };
		Token_stream ts;
		std::mutex req_mtx;//Мутеск 
		//Защищает данны при работе потока get_data и основного
		void(*callback)(void) { nullptr }; //error_callback
		std::map<nickname_t, std::vector<Message>> chats;
		
	};
}
