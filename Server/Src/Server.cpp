#include "Server.h"



Nikson::Server::Server(u_short port)
    :listening{ make_server(AF_INET, port) }
{
    FUNC_LOG;
    FD_ZERO(&read_set);
}

void Nikson::Server::run()
{
    FUNC_LOG;
    FD_SET(listening,&read_set);

    while (running)
        tick();

}

void Nikson::Server::stop()
{
    FUNC_LOG;
    running = false;
}

void Nikson::Server::tick()
try{
    FUNC_LOG;
    fd_set acv = read_set;
    Select(acv.fd_count, &acv, nullptr, nullptr, nullptr);
    for (int i = 0; i < acv.fd_count; ++i) 
    {
        SOCKET s = acv.fd_array[i];
        if (s == listening) { reply_good(connect()); }
        else
            if (message(s))
                reply_good(s);
    }
}
catch (Nikson::Winsock_error& e) 
{
    FUNC_LOG;
    switch (e.where()) {
    case Where::send:
    case Where::recv:
        disconnect(e.who());
        return;
    case Where::FD_SET://fd_set переаолнен
        reply_bad(e.who(), "Server is full. Please, try again later.");
        return;

    default:
        std::cerr << e.what() << '\n';
        return;

    }
}
catch (Nikson::Bad& e)
{
    reply_bad(e.who(), e.what());
}
catch (Token::Invalid)
{
}
catch (...)
{

}
Nikson::client_t Nikson::Server::connect()
{
    FUNC_LOG;
    client_t client = Accept(listening, nullptr, nullptr);
    Fd_set(client, &read_set);
    return client;
}

void Nikson::Server::disconnect(client_t c)
{
    FUNC_LOG;
    Close(c);
    FD_CLR(c, &read_set);
    Account* acc = find_log(c);
    if (acc != not_found)//if found
    {
        acc->set_online(INVALID_SOCKET);
        logs.erase(c);
    }

}

void Nikson::Server::reply_good(client_t c) 
{
    FUNC_LOG;
    ts.zero();
    ts << Token{Kind::good};
    Send(c, ts.data(), ts.size(), 0);
}

void Nikson::Server::reply_bad(client_t c, const std::string& msg) 
{
    FUNC_LOG;
    ts.zero();
    ts << Token{Kind::bad} << Token{Kind::data, msg};
    Send(c, ts.data(), ts.size(), 0);

}



bool Nikson::Server::message(client_t c)
{
    FUNC_LOG;
    char ster[SERVER_BUFF];
    ZeroMemory(ster, sizeof ster);

    int size = Recv(c, ster, sizeof ster, 0);
    constexpr int fin_pock = 0;

    if (size == fin_pock) { disconnect(c); return no_reply; }

    ts.load(ster, size);

    Token t = ts.get();
    if (t.kind() == Kind::debug)
    {
        /*
        * Режим debug удобный для тестирования с Putty
        * Раньше использовалось goto(с которым код выглядел более однородным)
        * Но в связи что debug так сильно отличается от других сообщений
        * Принято вывести его за границы switch
        * + Не будет рекурсивного вызова debug шутниками
        */
        debug(c);
        t = ts.get();
    }

    switch (t.kind()) 
    {
    case Kind::create_acc:
        create_account(c);
        break;
    case Kind::delete_acc:
        delete_account(c);
        break;
    case Kind::log_in:
        log_in(c);
        break;
    case Kind::log_out:
        log_out(c);
        break;
    case Kind::send_to:
        send_to(c);
        break;
    case Kind::get_data:
        get_data(c);
        break;
    case Kind::change_password:
        change_password(c);
        break;
    // Здесь был debug 
    default:
        bad(c, "Unknow message kind");
    }
    return true;
}

void Nikson::Server::create_account(client_t c)
{
    FUNC_LOG;
    assert(find_log(c) == not_found, c, "Please log out before creating new account");
    nickname_t nn = nickname(c);
    assert(find_account(nn) == not_found, c, "Nickname is taken");
    password_t pw = password(c);

    accounts[nn] = std::make_shared<Account>(nn,pw);
}

void Nikson::Server::delete_account(client_t c)
{
    FUNC_LOG;
    Account* acc = find_log(c);
    assert(acc!=not_found&&
        check_password(acc,password(c)), c, "Wrong password or not logged in");
    logs.erase(c);
    accounts.erase(acc->nickname());
}

void Nikson::Server::log_in(client_t c)
{
    FUNC_LOG;
    assert(find_log(c) == not_found, c, "Client has already logged in");
    nickname_t nn = nickname(c);
    Account* acc = find_account(nn);
    assert(acc != not_found && !acc->online()&& check_password(acc, password(c)),
        c, "Wrong nickname or wrong password or account in use");

    logs[c] = accounts[nn];
    acc->set_online(c);
    get_data(c);

}

void Nikson::Server::log_out(client_t c)
{
    FUNC_LOG;
    Account* log = find_log(c);
    assert(log != not_found, c, "Not logged in");
    logs.erase(c);
    log->set_online(INVALID_SOCKET);
}

void Nikson::Server::send_to(client_t c)
{
    FUNC_LOG;
    Account* sndr = find_log(c);
    assert(sndr != not_found, c, "Not logged in");
    Account* rcvr = find_account(nickname(c));
    assert(rcvr != not_found, c, "Receiver not found");
    /*assert(rcvr->nickname() != sndr->nickname(), c, "A typo? Sending data to yourself?");*/
    if (rcvr->online())
    {
        instant_send(rcvr->get_socket(), sndr->nickname(), data(c));
    }
    else rcvr->save_data(sndr, data(c));


}

void Nikson::Server::instant_send(client_t to, nickname_t from, const data_t& dt)
{
    ts.zero();
    ts << Token{ Kind::data, from } << Token{ Kind::data, dt };
    Send(to, ts.data(), ts.size(), 0);
}

void Nikson::Server::get_data(client_t c)
{
    FUNC_LOG;
    Account* acc = find_log(c);
    assert(acc != not_found, c, "Not logged in");
    for (auto& msg : acc->open_data()) {
        ts.zero();
        ts << Token{Kind::data, msg.first} << Token{Kind::data, msg.second};
        Send(c, ts.data(), ts.size(), 0);

    }
    acc->zero();

}
void Nikson::Server::change_password(client_t c)
{
    FUNC_LOG;
    Account* acc = find_log(c);

    assert(acc != not_found &&check_password(acc,password(c)), c, "Not logged in or wrong password");
    password_t new_pw = password(c);
    acc->set_password(new_pw);
    
}
#include <sstream>
void Nikson::Server::debug(client_t c)
{
    FUNC_LOG;
    std::string txt{char(Kind::debug) + std::string{ts.data(), static_cast<uint64_t>(ts.size())}};
    ts.zero();
    
    std::istringstream ss{txt};
    
    std::vector<Token> tt;
    for (Token t; ss >> t;)
        ts << t, tt.push_back(t);
    if (ss.eof()) return;//Отлично достигнут конец файла
    if (ss.fail()) bad(c, "Bad debug formatting");
    if (ss.bad()) bad(c, "Server error while formatting debug mesaage");
        
    


    
}

Nikson::Account* Nikson::Server::find_log(client_t c) const
{
    FUNC_LOG;
    auto acc = logs.find(c);
    if(acc==logs.end()) return nullptr;
    LOG(*(acc->second));
    return acc->second.get();
}

Nikson::Account* Nikson::Server::find_account(const nickname_t& nn) const
{
    FUNC_LOG;
    auto acc = accounts.find(nn);
    if (acc == accounts.end()) return nullptr;
    LOG(*(acc->second));
    return acc->second.get();
}

Nikson::nickname_t Nikson::Server::nickname(client_t c)
{
    FUNC_LOG;
    nickname_t nn = data(c);
    assert(is_nickname(nn), c, "Bad nickname symbols");
    tolower(nn);
    return nn;
}

Nikson::password_t Nikson::Server::password(client_t c)
{
    FUNC_LOG;
    password_t pw = data(c);
    assert(is_password(pw), c, "Bad password symbols");
    return pw;
}

Nikson::data_t Nikson::Server::data(client_t c)
{
    FUNC_LOG;
    Token dt= ts.get();
    assert(dt.kind() == Kind::data, c, "Bad data");
    return dt.list();
}

Nikson::Bad::Bad(client_t c, const std::string& m)
    :cl{c},msg{m}
{
    FUNC_LOG;
}

Nikson::Account::Account(nickname_t n, password_t p)
    :nn{ n }, pw{ p }
{
    FUNC_LOG;
}

void Nikson::Account::save_data(Account* sndr, const data_t& dt)
{
    FUNC_LOG;
    cloud.emplace_back(sndr->nickname(), dt);
}




#include <iomanip>
std::ostream& Nikson::operator<<(std::ostream& ost, const Account& acc)
{
    std::string nn, pw;
    for (char n : acc.nickname())
        nn += n;
    for (char p : acc.password())
        pw += p;
    using namespace std;
    ost << '|' << setw(10) << nn << '|' << setw(10) << pw << "|\n";
    ost << '|' << setw(10) <<(acc.online()?"online":"offline") << '|' << setw(10) << acc.count() << '|';
    ost << acc.get_socket() << '\n';
    return ost;
}
