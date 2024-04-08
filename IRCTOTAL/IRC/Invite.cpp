#include "Server.hpp"
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>

void	Server::invite_pars(Client *client, std::string buffer)
{
	size_t cmdEnd = buffer.find_first_of(" \t", 0);
	if (cmdEnd == std::string::npos)
		throw std::string("INVITE: /INVITE user to #channel_name");
	size_t userStart = buffer.find_first_not_of(" \t\r\n", cmdEnd);
	if (userStart == std::string::npos)
		throw std::string("INVITE: /INVITE user to #channel_name");
	size_t userEnd = buffer.find_first_of(" \t", userStart);
	if (userEnd == std::string::npos)
		throw std::string("INVITE: /INVITE user to #channel_name");
	std::string user = buffer.substr(userStart, userEnd - userStart);
	size_t channelStart = buffer.find_first_not_of(" \t#\r\n", userEnd);
	if (channelStart == std::string::npos)
		throw std::string("INVITE: /INVITE user to #channel_name");
	size_t channelEnd = buffer.find_first_of(" \t\r\n", channelStart);
	if (channelEnd == std::string::npos)
		throw std::string("INVITE: /INVITE user to #channel_name");
	std::string channel = buffer.substr(channelStart, channelEnd - channelStart);
	size_t checkRest = buffer.find_first_not_of(" \t\r\n", channelEnd);
	if (checkRest != std::string::npos)
		throw std::string("INVITE: /INVITE user to #channel_name");
	invite_exec(client, user, channel);
}

void	Server::invite_exec(Client *client, std::string user, std::string channel)
{
	Channel	*tmp = NULL;
	Client	*tmp2 = NULL;

	if (_channel.find(channel) != _channel.end())
		tmp = _channel[channel];
	std::map<int, Client *>::iterator it = _client.begin();
	while(it != _client.end())
	{
		if (it->second->getUsername() == user)
		{
			tmp2 = it->second;
		}
		it++;
	}

	if (tmp == NULL)
		throw std::string("INVITE: Channel does not exist !");
	if (tmp2 == NULL)
		throw std::string("INVITE: User does not exist !");

	std::map<std::string, Client *> admins = tmp->getAdmins();
	std::map<std::string, Client *> clients = tmp->getClients();
	std::map<std::string, Client *>::iterator it1 = admins.find(client->getNickname());
	std::map<std::string, Client *>::iterator it2 = clients.find(client->getNickname());
	std::map<std::string, Client *>::iterator it3 = admins.find(user);
	std::map<std::string, Client *>::iterator it4 = clients.find(user);

	if (it1 == admins.end() && it2 == clients.end())
		throw std::string("INVITE: Client is not in the channel !");
	if (it3 != admins.end() || it4 != clients.end())
		throw std::string("INVITE: User is already in the channel !");
	// send the invitation to the user
}