#include "Tokens.h"

#include <iostream>
Nikson::Token Nikson::Token_stream::get()
{
	if (full) {
		full = false;
		return buffer;
	}
	Kind kind = Kind(get_char());
	switch (kind) 
	{
	case Kind::create_acc:
	case Kind::delete_acc:
	case Kind::log_in:
	case Kind::log_out:
	case Kind::send_to:
	case Kind::get_data:
		case Kind::debug:
		case Kind::good:case Kind::bad:
		case Kind::change_password:
		return Token{ kind };
	case Kind::data:
	{
		std::vector<char> v;
		int size = get_int();
		if (size > st.size())
			throw Token_stream::Error{};
		for (int i = 0; i < size; ++i)
			v.push_back(get_char());
		return Token{ kind,v };
	}
	default:
		throw Token::Invalid{};
	}

}

void Nikson::Token_stream::unget(Token t)
{
	if (full)
		throw Token_stream::Error{};
	buffer = t;
	full = true;
}

void Nikson::Token_stream::put(Token t)
{
	Kind kind = t.kind();
	switch (kind) {
	case Kind::create_acc:
	case Kind::delete_acc:
	case Kind::log_in:
	case Kind::log_out:
	case Kind::send_to:
	case Kind::get_data:
	case Kind::debug:
	case Kind::good:case Kind::bad:
	case Kind::change_password:
		put_char(char(kind));
		return;
	case Kind::data:
	{
		put_char(char(kind));
		put_int(static_cast<int>(t.list().size()));
		for (char c : t.list())
			put_char(c);
		return;
	}
	
	
	default:
		zero();
		throw Token::Invalid{};
	}
}

void Nikson::Token_stream::load(char* source, int size)
{
	st.clear();
	st.reserve(size);
	for (int i = 0; i < size; ++i)
		st.push_back(*(source + i));
	return;
}

void Nikson::Token_stream::zero()
{
	st.clear();
}


Nikson::Token_stream& Nikson::Token_stream::operator<<(const Token& t)
{
	put(t);
	return *this;
}

Nikson::Token_stream& Nikson::Token_stream::operator>>(Token& t)
{
	t = get();
	return *this;
}

char Nikson::Token_stream::get_char()
{
	if (st.empty())
		throw Token_stream::Error{};
	char ch = st[0];
	st.erase(st.begin());
	return ch;
}

int Nikson::Token_stream::get_int()
{
	if (st.size() < 4)
		throw Token_stream::Error{};
	int n;
	memcpy(&n, st.data(), sizeof n);

	st.erase(st.begin(), st.begin() + 4);
	return n;
}

void Nikson::Token_stream::put_char(char c)
{
	st.push_back(c);

}

void Nikson::Token_stream::put_int(int n)
{
	char chs[4];
	memcpy(chs, &n, sizeof n);
	for (char c : chs)
		st.push_back(c);
}

Nikson::Token::Token(Kind kind, const std::vector<char>& list)
	:k{kind},l{list}
{}

Nikson::Token::Token(Kind kind, const std::string& text)
	:k{ kind }
{
	init_vector(text, l);
}

Nikson::Token::Token(Kind kind)
	:k{kind}
{
}

bool Nikson::is_nickname(const std::vector<char>& nn)
{
	if (nn.size() > max_nickname_len())
		return false;

	for (int i = 0; i < nn.size(); ++i)
	{
		char c = nn[i];
		if (isalpha(c) || (isdigit(c) && i > 0) || c == '_');
		else return false;

	}
	return true;
}

bool Nikson::is_password(const std::vector<char>& pw)
{
	if (pw.size() > max_password_len())
		return false;

	for (int i = 0; i < pw.size(); ++i)
	{
		char c = pw[i];
		if (!isgraph(c))return false;

	}
	return true;
}

void Nikson::tolower(nickname_t& nn)
{
	for (char& c : nn)
		c = std::tolower(c);
}

void Nikson::init_vector(const std::string& s,std::vector<char>& v)
{
	v = {};
	for (char c : s)
		v.push_back(c);
}
std::string Nikson::make_string(const std::vector<char>& v)
{
	return std::string{v.begin(), v.end()};
}
#include <map>
#include <string>
#include <iostream>
std::istream& Nikson::operator>>(std::istream& is, Token& t)
{
	using namespace std;

	static map <string, Kind> tbl =
	{
		{"create_acc",Kind::create_acc},{"delete_acc",Kind::delete_acc},
		{"log_in",Kind::log_in},{"log_out",Kind::log_out},
		{"send_to",Kind::send_to},{"get_data",Kind::get_data},
		{"data", Kind::data},{"change_password",Kind::change_password},
		//Сокращения
		{"ca",Kind::create_acc},{"da",Kind::delete_acc},
		{"li",Kind::log_in},{"lo",Kind::log_out},
		{"st",Kind::send_to},{"gd",Kind::get_data},
		{"d", Kind::data},{"cp",Kind::change_password}
	};
	char ch=0;
	is >> ch;
	if (!is)
		return is;
	switch (ch) {
	case char(Kind::debug):
	{
		string keyword;
		is >> keyword;
		auto kind = tbl.find(keyword);
		if (kind == tbl.end())//NO
		{
			//string data nickname or password
			is.clear(ios_base::failbit);
			return is;
		}
		switch (kind->second)
		{
		case Kind::data:
		{
			char c;
			while (is.get(c) && c == ' ');// Съедаем ' ' до предложения
			if (!is) return is;
			is.putback(c);

			string line;
			if (!getline(is, line))
				return is;
			while (!line.empty()&&(line.back() == '\r'
				|| line.back() == '\n'
				|| line.back() == ' '))
				line.pop_back();
			t = Token{ Kind::data,line };
			return is;
		}
		default:
			t = Token{ kind->second };
			return is;
		}
	}
	default:
	{
		is.putback(ch);
		string dt;
		is >> dt;
		t = Token{ Kind::data,dt };
		return is;
	}
	}

}




