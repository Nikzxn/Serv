/*
* TCP	Server
* Автор:Nikson
* Дата: 2 августа 2023
* Грамматика:
* Сообщение:
*	Создать_акк
*	Удалить_акк
*	Войти
*	Выйти
*	Отправить
*	Принять_сообщения
* Север работает по NRP (Nikson Resulting Protocol)
* 
* Client --[MSG]--> Server
* ^				     |
* *---------[RES]----*
* Client отправляет сообщение Серверу; Сервер отвечает на него с good или bad
*/

#pragma once
#include "Nikson.h"
#include "Tokens.h"
#include <iostream>
#include <map>
#ifndef SERVER_BUFF
	#define SERVER_BUFF BUFF_SIZE
#endif
#ifdef _DEBUG
	#define LOG(msg) std::clog<<msg<<'\n'
	#define FUNC_LOG std::clog<<__func__<<'\n'
#else
	#define LOG(msg)
	#define FUNC_LOG
#endif

namespace Nikson {
	using client_t = SOCKET;//Для создания понятности
	class Account 
	{
	public:
		using named_data_t = std::pair<nickname_t, data_t>;
		Account(nickname_t, password_t);
		~Account() { LOG(make_string(nn) + " is dying :("); }

		const password_t& password()const  { return pw; }
		const nickname_t& nickname()const { return nn; }
		void save_data(Account* sndr, const data_t& dt);
		const std::vector< named_data_t>& 
			open_data() const { return cloud; }

		int count() const { return static_cast<int>(cloud.size()); }
		void set_password(const password_t& p) { pw = p; };
		void set_online(client_t c)  { socket = c; }
		bool online()const { return socket!= INVALID_SOCKET; }
		void zero() { cloud.clear(); }
		client_t get_socket() const{ return socket; }
	private:
		nickname_t nn;
		password_t pw;
		client_t socket{ INVALID_SOCKET };
		std::vector<named_data_t> cloud;
		bool o{ false };
	};//Account
	std::ostream& operator<<(std::ostream& ost, const Account& acc);

	constexpr Account* not_found{ nullptr };
	constexpr bool no_reply{ false };

	inline bool check_password(const Account* acc, const password_t& pw) { return acc->password() == pw; }
	
	class Bad
	{
	public:
		Bad(client_t c, const std::string& m);
		const std::string& what() const { return msg; }
		client_t who() const { return cl; }
	private:
		client_t cl;
		std::string msg;
	};
	inline void bad(client_t c, const std::string& m) { throw Bad{c, m}; }
	inline void assert(bool cond, client_t c,const std::string& s) { if (!(cond)) bad(c,s); }
	class Server
	{
	public:
		
		Server(u_short port);
		void run();
		void stop();
	private:
		void tick();
	private:
		//Отправка успешен ли результат
		void reply_good(client_t c);
		void reply_bad(client_t c, const std::string& msg);

		client_t connect	();//@return: новый сокет
		void disconnect	(client_t c);
		//Обработка грамматики
		bool message		(client_t c);//@return true если нужно ответить
		void create_account	(client_t c);
		void delete_account	(client_t c);
		void log_in			(client_t c);
		void log_out		(client_t c);
		void send_to		(client_t c);
		void instant_send   (client_t to, nickname_t from, const data_t& dt);
		void get_data		(client_t c);
		void change_password(client_t c);
	
		void debug			(client_t c);
		/*
		* Анализ удобной для тестирования грамматики
		* Пример:
		*	"/ca nikson secret" -> 3 токена{ создать_акк, никнайм, пароль}
		* Виды сообщений:
		*	/ca -- создать аккаунт
		*	/da -- удалить аккаунт
		* 	/li -- создать аккаунт
		*	/da -- удалить аккаунт
		* 	/ca -- создать аккаунт
		*	/da -- удалить аккаунт
		{"ca",Kind::create_acc},{"da",Kind::delete_acc},
		{"li",Kind::log_in},{"lo",Kind::log_out},
		{"st",Kind::send_to},{"gd",Kind::get_data},
		{"d", Kind::data},{"cp"
		* 
		* 
		*/



		Account* find_log(client_t c) const;
		Account* find_account(const nickname_t& c) const;
		nickname_t nickname(client_t c);
		password_t password(client_t c);
		data_t	   data	   (client_t c);
	private:
		bool running{true};
		fd_set read_set = {};
		SOCKET listening{ INVALID_SOCKET };
		std::map<client_t, std::shared_ptr<Account>> logs;
		std::map<nickname_t, std::shared_ptr<Account>> accounts;
		Token_stream ts;
	};//Server
}

