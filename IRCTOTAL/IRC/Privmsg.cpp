#include "Server.hpp"
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>

void	Server::privmsg_pars(Client *client, std::string buffer)
{
	size_t cmdEnd = buffer.find_first_of(" \t", 0);
	if (cmdEnd == std::string::npos)
		throw std::string("PRIVMSG: /msg user message");
	size_t userStart = buffer.find_first_not_of(" \t\r\n", cmdEnd);
	if (userStart == std::string::npos)
		throw std::string("PRIVMSG: /msg user message");
	size_t userEnd = buffer.find_first_of(" \t", userStart);
	if (userEnd == std::string::npos)
		throw std::string("PRIVMSG: /msg user message");
	std::string user = buffer.substr(userStart, userEnd - userStart);
	size_t msgStart = buffer.find_first_not_of(" \t\r\n", userEnd);
	if (msgStart == std::string::npos)
		throw std::string("PRIVMSG: /msg user message");
	size_t msgEnd = buffer.size();
	while (msgEnd > 0 && (buffer[msgEnd] == ' ' ||  buffer[msgEnd] == '\t'
		|| buffer[msgEnd] == '\r' || buffer[msgEnd] == '\n'))
		msgEnd--;
	std::string	msg = buffer.substr(msgStart, msgEnd - msgStart);
	privmsg_exec(client, user, msg);
}

void	Server::privmsg_exec(Client *client, std::string user, std::string msg)
{
	(void)client;
	Client	*tmp = NULL;
	std::map<int, Client *>::iterator it = _client.begin();

	for (it = _client.begin(); it != _client.end(); it++)
	{
		tmp = it->second;
		if (tmp->getNickname() == user)
			break ;
	}
	if (it == _client.end())
		throw std::string("PRIVMSG: user does not exist !");
	send(tmp->getSocket(), msg.c_str(), msg.size(), 0);
}
