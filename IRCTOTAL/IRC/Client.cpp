#include "Client.hpp"

Client::Client(){}

Client::~Client() {}

Client::Client(int fd, sockaddr_in addr) :  _address(addr), _socket_fd(fd), _register(false) , _check(0){}

std::string Client::getPass()
{
    return (_pass);
}
std::string Client::getUsername()
{
    return(_username);
}
std::string Client::getNickname()
{
    return(_nickname);
}
std::string Client::getRealname()
{
    return (_realname);
}
Client *Client::getClient()
{
    return(this);
}
bool Client::getRegister()
{
    return(_register);
}

int Client::getSocket()
{
    return (_socket_fd);
}


void    Client::setPassword(std::string pass) {
    _pass = pass;
}

void Client::setNickname(std::string nick)
{
    _nickname = nick;
}

void Client::setUsername(std::string username)
{
        _username = username;
}

void Client::setRegister(bool value)
{
    _register = value;
}

void Client::checkplus()
{
    _check++;
}

int Client::getCheck()
{
    return(_check);
}