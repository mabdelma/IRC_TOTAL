#include "Channel.hpp"

Channel::~Channel()
{
    _clients.clear();
    _admins.clear();
}

std::map<std::string, Client *> Channel::getAdmins()
{
    return (_admins);
}
std::map<std::string, Client *> Channel::getClients()
{
    return(_clients);
}

Channel::Channel(Client *client, std::string channelname, std::string password)
{
    _name = channelname;
    _password = password;
    _needpass = true;
    _clients[client->getNickname()] = client;
    _admins[client->getNickname()] = client;
}

Channel::Channel(std::string channelname, std::string password)
{
    _name = channelname;
    _password = password;
    _needpass = false;
}
std::string Channel::getName()
{
    return(_name);
}
std::string Channel::getPassword()
{
    return(_password);
}

std::string Channel::getTopic()
{
    return (_topic);
}

void    Channel::setTopic(const std::string &newTopic)
{
    _topic = newTopic;
}

void    Channel::setInvitation(bool invitation)
{
    _needinvitation = invitation;
}

void    Channel::setAdmintopic(bool admintopic)
{
    _admintopic = admintopic;
}

void    Channel::setPass(bool pass)
{
    _needpass = pass;
}

void    Channel::setLimitadmin(bool limit)
{
    _limitclients = limit;
}

bool       Channel::getNeed()
{
    return(_needpass);
}
void       Channel::addClients(Client *clients)
{
    if(_clients.find(clients->getUsername()) == _clients.end())
    {
        _clients[clients->getUsername()] = clients;
    }
}

void       Channel::addAdmins(Client *admins)
{
    if(_admins.find(admins->getUsername()) == _admins.end())
    {
        _admins[admins->getUsername()] = admins;
    }
}
void       Channel::rmClients(std::string clientname)
{
    std::map<std::string , Client *>::iterator it = _clients.find(clientname);
    if (it != _clients.end())
    {
        delete it->second;
        _clients.erase(it);
    }
}
void       Channel::rmAdmins(std::string adminname)
{
    std::map<std::string , Client *>::iterator it = _admins.find(adminname);
    if (it != _admins.end())
    {
        delete it->second;
        _admins.erase(it);
    }
}

void Channel::print() {
    std::cout << "Channel name: " << _name << std::endl;
    std::cout << "Password: " << (_needpass ? _password : "no password") << std::endl;
    std::cout << "Clients: ";
    std::map<std::string, Client *>::iterator it;
    for (it = _clients.begin(); it != _clients.end(); ++it) {
        std::cout << it->first << " ";
    }
    std::cout << std::endl;
    std::cout << "Admins: ";
    for (it = _admins.begin(); it != _admins.end(); ++it) {
        std::cout << it->first << " ";
    }
    std::cout << std::endl;
}