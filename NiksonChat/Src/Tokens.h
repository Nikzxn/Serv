#pragma once
#include <vector>
#include <string>
namespace Nikson {
	enum class Kind :char
	{
		none = -1,
		create_acc = 'C',		//[C->S] {create_acc,data,data} 
		delete_acc = 'D',		//[C->S] {create_acc,data} 
		log_in = 'I',		//[C->S]     {create_acc,data,data} 
		log_out = 'O',		//[C->S]     {log_out}
		send_to = '>',		//[C->S]     {send_to,data,data}
		get_data = '<',		//[C->S]     {get_data}
		data = '?',                //{data{len:int,char_array}}
		good = '+',		//[S->C]         {good}
		bad = '-',		//[S->C]     {bad,data}
		change_password='@',
		debug='/'
		
	};

	using nickname_t = std::vector<char>;
	using password_t = std::vector<char>;
	using data_t = std::vector<char>;
	inline int max_nickname_len() {return 16;}
	inline int max_password_len() {return 64;}
	bool is_nickname(const std::vector<char>& nn);
	bool is_password(const std::vector<char>& pw);
	void tolower(nickname_t& nn);
	void init_vector(const std::string& s, std::vector<char>& v);
	std::string make_string(const std::vector<char>& v);
	
	class Token
	{
	public:
		class Invalid {};
		Token() = default;
		Token(Kind kind, const std::vector<char>& list);
		Token(Kind kind, const std::string& text);
		Token(Kind kind);
		Kind kind() const{return k;}
		const std::vector<char>& list() const { return l; }
	private:
		Kind k=Kind::none;//kind
		std::vector<char> l;//array
	};//Token
	std::istream& operator>>(std::istream& is, Token& t);


	class Token_stream {
	public:
		class Error{};
		Token_stream() = default;

		Token get();//Возвращает лексему с переди
		void unget(Token t);
		void put(Token t);					//Добавляет лексему взад
		void load(char* source, int size);	//Загружает size байтов из source в ster
		void zero();
		const char* data() const { return st.data(); }
		int size() const { return static_cast<int>(st.size()); }
		Token_stream& operator<<(const Token& t);
		Token_stream& operator>>(Token& t);
	private:
		char get_char();
		int  get_int();
		void put_char(char);
		void put_int(int);
	private:
		std::vector<char> st;
		bool full{ false };
		Token buffer;
	};
	
}

