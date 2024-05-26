
#include "Server.hpp"
#include "Command.hpp"
#include "Client.hpp"

Command::Command(void)
{

	this->message = "";
	
}

Command::~Command(void)
{

}



std::vector<std::string> Command::ft_split(std::string str, char delimiter)
{
	std::vector<std::string> substrings;
	std::string substring = "";
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] != delimiter)
		{
			substring += str[i];
		}
		else
		{
			substrings.push_back(substring);
			while (str[i] == delimiter)
				i++;
			i--;
			substring = "";
		}
	}
	substrings.push_back(substring);
	return (substrings);
}

std::vector<Channel>::iterator Command::chan_exist(std::string channel)
{
	for (this->chan_it = Server::_channels.begin(); this->chan_it != Server::_channels.end(); chan_it++)
	{
		if (chan_it->getName() == channel)
			return chan_it;
	}
	return chan_it;
}

std::vector<Client>::iterator Command::client_exist(std::string nick)
{
	for (this->client_it = Server::_clients.begin(); this->client_it != Server::_clients.end(); client_it++)
	{
		if (client_it->nickName == nick)
			return client_it;
	}
	return client_it;
}

void Command::join(std::string channel_s, std::string key_s, Client client)
{
	std::vector<std::string> channel_split = ft_split(channel_s, ',');
	std::vector<std::string> key_split = ft_split(key_s, ',');
	std::vector<Channel>::iterator it;
	std::vector<Client>::iterator it_i;
	std::vector<std::string>::iterator it_s;
	std::vector<std::string>::iterator it_k = key_split.begin();
	
	for (it_s = channel_split.begin(); it_s != channel_split.end(); it_s++)
	{
		if ((it_s->at(0) != '#' && it_s->at(0) != '&'))
			send(client._fd, INVALID_CHAN, strlen(INVALID_CHAN), 0);
		else if(it_s->size() <= 1)
			send(client._fd, INVALID_CHAN_NAME, strlen(INVALID_CHAN_NAME), 0);
		else
		{
			it = chan_exist(*it_s);
			if (it != Server::_channels.end())
			{
				if (it->isclient(client))
				{
					sendErrorMessage(client._fd, (client.nickName + " " + it->getName() + YES_CLIENT_M), ERR_clientONCHANNEL);
					return ;
				}
				if (it_k != key_split.end() && key_s != "")
				{
					if (it->isMode('k') == true)
					{
						if (*it_k == it->getPass())
						{
							if (it->isMode('i') == true)
							{
								if (it->isInvited(client))
								{
									it_i = it->inv_in_chan(client._fd);
									if (it_i != it->invites.end())
										it->invites.erase(it_i);
									it->addclient(client);
								}
								else
									sendErrorMessage(client._fd, (it->getName() + NO_INV_M), ERR_INVITEONLYCHAN);
							}
							else
								it->addclient(client);
						}
						else
							sendErrorMessage(client._fd, (it->getName() + NO_KEY_M), ERR_BADCHANNELKEY);
					}
					else
						sendErrorMessage(client._fd, "Key Not required to join channel\n", ERR_BADCHANNELKEY);
					it_k++;
				}
				else
				{
					if (it->isMode('i') == true)
					{
						if (it->isInvited(client))
						{
							if (it->isMode('k') == true)
								sendErrorMessage(client._fd, (it->getName() + NO_KEY_M), ERR_BADCHANNELKEY);
							else
							{
								it_i = it->inv_in_chan(client._fd);
								if (it_i != it->invites.end())
									it->invites.erase(it_i);
								it->addclient(client);
							}
						}
						else
							sendErrorMessage(client._fd, (it->getName() + NO_INV_M), ERR_INVITEONLYCHAN);
					}
					else
					{
						if (it->isMode('k') == true)
								sendErrorMessage(client._fd, (it->getName() + NO_KEY_M), ERR_BADCHANNELKEY);
						else
							it->addclient(client);
					}
				}
			}
			else
			{
				if (it_k != key_split.end())
				{
					Channel new_channel(*it_s, *it_k);
					new_channel.addclient(client);
					Server::_channels.push_back(new_channel);
					it_k++;
				}
				else
				{
					Channel new_channel(*it_s, "");
					new_channel.addclient(client);
					Server::_channels.push_back(new_channel);
				}
			}
		}
	}
}

void Command::kick(std::string channel, std::string client_kick, const std::vector<std::string>& splitmsg, Client client)
{
	std::vector<Channel>::iterator it_c;

	it_c = chan_exist(channel);
	if (it_c != Server::_channels.end())
		it_c->kickclient(client_kick, splitmsg, client);
	else
		sendErrorMessage(client._fd, (channel + NO_CHAN_M), ERR_NOSUCHCHANNEL);
}

void Command::invite(std::string client, std::string channel, Client client_o)
{
	std::vector<Channel>::iterator it_c;
	std::vector<Client>::iterator it_s;

	it_c = chan_exist(channel);
	if (it_c != Server::_channels.end())
	{
		it_s = client_exist(client);
		if (it_s != Server::_clients.end())
		{
			if (it_c->isOperator(client_o) != 1)
				sendErrorMessage(client_o._fd, OP_ERR_M, ERR_CHANOPRIVSNEEDED);
			else
			{
				if (it_c->isclient(*it_s))
					sendErrorMessage(client_o._fd, (client + " " + channel + YES_CLIENT_M), ERR_clientONCHANNEL);
				else
				{
					if (it_c->isMode('i') == true)
					{
						if (it_c->isInvited(*it_s))
							send(client_o._fd, "client is already invited\n", strlen("You are already invited\n"), 0);
						else
						{
							message = "You're invited to the Channel " + channel + " \n";
							send(it_s->_fd, message.c_str(), strlen(message.c_str()), 0);
							it_c->invites.push_back(*it_s);
						}
					}
					else
						send(client_o._fd, "Channel is not on +i mode\n", strlen("Channel is not on +i mode\n"), 0);
				}
			}
		}
		else
			sendErrorMessage(client_o._fd, (client + NO_CLIENT_M), ERR_NOSUCHNICK);
	}
	else
		sendErrorMessage(client_o._fd, (channel + NO_CHAN_M), ERR_NOSUCHCHANNEL);
	
}

void Command::topic(std::string channel, std::string topic, Client client)
{
	std::vector<Channel>::iterator it_c;

	it_c = chan_exist(channel);
	if (it_c != Server::_channels.end())
	{
		if (it_c->isMode('t') == true)
		{
			if (it_c->isOperator(client) == true)
			{
				if (topic != "")
				{
					it_c->setTopic(topic);
					message = "Topic of channel " + channel + "changed." + "\n";
					send(client._fd, message.c_str(), strlen(message.c_str()), 0);
				}
				else
				{
					message = "Channel " + channel + "'s topic: " + it_c->getTopic() + "\n";
					send(client._fd, message.c_str(), strlen(message.c_str()), 0);
				}
			}
			else
				sendErrorMessage(client._fd, OP_ERR_M, ERR_CHANOPRIVSNEEDED);
		}
		else
		{
			if (topic != "")
			{
				it_c->setTopic(topic);
				message = "Topic of channel " + channel + " changed." + "\n";
				send(client._fd, message.c_str(), strlen(message.c_str()), 0);
			}
			else
			{
				message = "Channel " + channel + "'s topic: " + it_c->getTopic() + "\n";
				send(client._fd, message.c_str(), strlen(message.c_str()), 0);
			}
		}
	}
	else
		sendErrorMessage(client._fd, (channel + NO_CHAN_M), ERR_NOSUCHCHANNEL);
}

void Command::privmsg(std::string reciever, const std::vector<std::string>& splitmsg, Client client)
{
	std::vector<std::string> reciever_split = ft_split(reciever, ',');
	std::vector<Channel>::iterator it_c;
	std::vector<Client>::iterator it_u;
	std::vector<std::string>::iterator it_s;
	unsigned long i = 2;

	for (it_s = reciever_split.begin(); it_s != reciever_split.end(); it_s++)
	{
		it_u = client_exist(*it_s);
		if (it_u == Server::_clients.end())
		{
			it_c = chan_exist(reciever);
			if (it_c != Server::_channels.end())
			{
				if (it_c->isclient(client))
				{
					std::vector<Client> temp_clients = it_c->getclients();
					for(std::vector<Client>::iterator it = temp_clients.begin(); it != temp_clients.end(); ++it)
					{
						if(it->_fd != client._fd)
						{
							send(it->_fd, (client.nickName + " :" ).c_str(), strlen((client.nickName + " :" ).c_str()), 0);
							while (i < splitmsg.size())
							{
								send(it->_fd, (splitmsg.at(i)).c_str(), strlen((splitmsg.at(i)).c_str()), 0);
								send(it->_fd, " ", strlen(" "), 0);
								i++;
							}
							send(it->_fd, "\n", strlen("\n"), 0);
							i = 2;
						}
					}
				}
				else
					sendErrorMessage(client._fd, (it_c->getName() + NOT_CHAN_USR), ERR_CANNOTSENDTOCHAN);
			}
		}
		else
		{
			if(client._fd == it_u->_fd)
				send(it_u->_fd, "can't send message to same client\n", strlen("can't send message to same client\n"), 0);
			else
			{
				send(it_u->_fd, (client.nickName + " :" ).c_str(), strlen((client.nickName + " :" ).c_str()), 0);
				while (i < splitmsg.size())
				{
					send(it_u->_fd, (splitmsg.at(i)).c_str(), strlen((splitmsg.at(i)).c_str()), 0);
					send(it_u->_fd, " ", strlen(" "), 0);
					i++;
				}
				send(it_u->_fd, "\n", strlen("\n"), 0);
				i = 2;
			}
		}
		if (it_u == Server::_clients.end() && it_c == Server::_channels.end())
			sendErrorMessage(client._fd, (*it_s + " :No such nickname" + " or channel.\n"), ERR_NOSUCHNICK);
	}
}

void Command::notice(std::string reciever, const std::vector<std::string>& splitmsg, Client client)
{
	std::vector<std::string> reciever_split = ft_split(reciever, ',');
	std::vector<Channel>::iterator it_c;
	std::vector<Client>::iterator it_u;
	std::vector<std::string>::iterator it_s;
	unsigned long i = 2;

	for (it_s = reciever_split.begin(); it_s != reciever_split.end(); it_s++)
	{
		it_u = client_exist(*it_s);
		if (it_u == Server::_clients.end())
		{
			it_c = chan_exist(reciever);
			if (it_c != Server::_channels.end())
			{
				if (it_c->isclient(client))
				{
					std::vector<Client> temp_clients = it_c->getclients();
					for(std::vector<Client>::iterator it = temp_clients.begin(); it != temp_clients.end(); ++it)
					{
						if(it->_fd != client._fd)
						{
							send(it->_fd, (client.nickName + " NOTICE: " ).c_str(), strlen((client.nickName + " NOTICE: " ).c_str()), 0);
							while (i < splitmsg.size())
							{
								send(it->_fd, (splitmsg.at(i)).c_str(), strlen((splitmsg.at(i)).c_str()), 0);
								send(it->_fd, " ", strlen(" "), 0);
								i++;
							}
							send(it->_fd, "\n", strlen("\n"), 0);
							i = 2;
						}
					}
				}
			}
		}
		else
		{
			if(client._fd != it_u->_fd)
			{
				send(it_u->_fd, (client.nickName + " NOTICE: " ).c_str(), strlen((client.nickName + " NOTICE: ").c_str()), 0);
				while (i < splitmsg.size())
				{
					send(it_u->_fd, (splitmsg.at(i)).c_str(), strlen((splitmsg.at(i)).c_str()), 0);
					send(it_u->_fd, " ", strlen(" "), 0);
					i++;
				}
				send(it_u->_fd, "\n", strlen("\n"), 0);
				i = 2;
			}
		}
	}
}

void Command::mode(std::string channel, std::string mode, Client client, std::string arg)
{
	std::vector<Channel>::iterator it_c;

	if (mode.size() != 2 && (mode[0] != '+' && mode[0] != '-'))
	{
		sendErrorMessage(client._fd, (mode + MODE_ERR_M), ERR_UNKNOWNMODE);
		return ;
	}
	it_c = chan_exist(channel);
	if (it_c == Server::_channels.end())
		sendErrorMessage(client._fd, (channel + NO_CHAN_M), ERR_NOSUCHCHANNEL);
	else
	{
		if (it_c->isOperator(client))
		{
			if (it_c->isMode(mode[1]) == 2)
				sendErrorMessage(client._fd, (mode + MODE_ERR_M), ERR_UNKNOWNMODE);
			else
					it_c->exec_mode(mode, client, arg);
		}
		else
			sendErrorMessage(client._fd, OP_ERR_M, ERR_CHANOPRIVSNEEDED);
	}
}

void Command::part(std::string channel, Client client)
{
	std::vector<Channel>::iterator it_c;
	std::vector<Client>::iterator it_u;

	it_c = chan_exist(channel);
	if (it_c == Server::_channels.end())
		sendErrorMessage(client._fd, (channel + NO_CHAN_M), ERR_NOSUCHCHANNEL);
	
	else
	{
		it_u = it_c->client_in_chan(client._fd);
		if (it_u != it_c->clients.end())
		{
			send(client._fd, ("You left the channel " + it_c->getName() + " \n").c_str(),
				strlen(("You left the channel " + it_c->getName() + " \n").c_str()), 0);
			it_c->clients.erase(it_u);
			it_u = it_c->op_in_chan(client._fd);
			if (it_u != it_c->operators.end())
				it_c->operators.erase(it_u);
			it_u = it_c->clients.begin();
			if (it_u != it_c->clients.end() && it_c->operators.size() == 0)
				it_c->operators.push_back(*it_u);
		}
		else
			sendErrorMessage(client._fd, (channel + NOT_CLIENT_M), ERR_NOTONCHANNEL);
	}
}