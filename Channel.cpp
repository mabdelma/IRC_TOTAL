#include "Server.hpp"

Channel::Channel(std::string str_n, std::string str_p)
{
	this->name = str_n;
	this->pass = str_p;
	this->client_limit = 0;
	this->topic = "";
	this->message = "";
	this->mode['i'] = false;
	this->mode['t'] = false;
	this->mode['k'] = false;
	this->mode['o'] = false;
	this->mode['l'] = false;
}

Channel::~Channel(void)
{

}

int	Channel::getclientLimit(void)
{
	return (client_limit);
}

std::string	Channel::getTopic(void)
{
	return (topic);
}

std::string	Channel::getPass(void)
{
	return (pass);
}

std::map<char, bool> Channel::getMode(void)
{
	return (mode);
}

std::vector<Client> Channel::getclients(void)
{
	return (clients);
}

std::vector<Client> Channel::getOperators(void)
{
	return (operators);
}

std::string Channel::getName(void) const
{
	return (name);
}



void Channel::setclientLimit(int num)
{
	client_limit = num;
}

void Channel::setTopic(std::string str)
{
	topic = str;
}

void Channel::setPass(std::string str)
{
	pass = str;
}

void Channel::setMode(char m, char sign)
{
	std::map<char, bool>::iterator it;
	for (it = this->mode.begin(); it != this->mode.end(); it++)
	{
		if (it->first == m)
		{
			if (sign == '+')
				it->second = true;
			else
				it->second = false;
		}
	}
}



void Channel::addclient(Client new_client)
{
	if (operators.size() == 0)
		operators.push_back(Client(new_client));
	if (this->isMode('l'))
	{
		if (this->client_len() == this->client_limit)
		{
			sendErrorMessage(new_client._fd, (this->getName() + CHAN_FULL), ERR_CHANNELISFULL);
			return ;
		}
	}
	clients.push_back(Client(new_client));
	std::string chan_message = "\n - WELCOME TO THE CHANNEL " + this->name + "! - \n";
	message = "List of Commands                                             Usage\n"
		"PRIVMSG - message client(s) in the channel           PRIVMSG <receiver>{,<receiver>} <text to be sent>\n"
		"MODE (o) - change the mode of the channel         MODE <channel> <+/-mode> {argument}\n"
		"TOPIC (o) - change the topic of the channel   œ     TOPIC <channel> <topic>\n"
		"INVITE (o) - invite another client to the channel   INVITE <nickname> <channel>\n"
		"KICK (o) - eject a client from a channel          KICK <channel> <client> [<comment>] \n\n";
	send(new_client._fd, (B_MUSTARD + chan_message + MUSTARD + message +  RESET).c_str(), strlen(message.c_str()) + strlen(chan_message.c_str())
		+ strlen(B_MUSTARD) + strlen(MUSTARD) + strlen(RESET), 0);
	
}

void Channel::kickclient(std::string client_kick, const std::vector<std::string>& splitmsg, Client client)
{
	std::vector<Client>::iterator it;
	std::vector<Client>::iterator it_s;
	std::vector<Client>::iterator it_o;
	unsigned long i = 3;

	for(it_s = this->clients.begin(); it_s != this->clients.end(); ++it_s)
	{
		if (it_s->nickName == client_kick)
		{
			if (this->isOperator(client) != 1)
			{
				sendErrorMessage(client._fd, OP_ERR_M, ERR_CHANOPRIVSNEEDED);
				return ;
			}
			else
			{
				if(client.nickName == client_kick)
				{
					sendErrorMessage(client._fd, "You cannot kick yourself\n", ERR_CANNOTSENDTOCHAN);
					return ;
				}
				send(it_s->_fd, "You have been kicked from the channel\n", strlen("You have been kicked from the channel\n"), 0);
				if (splitmsg.size() > 3)
					send(it_s->_fd, "Reason for kicking: ", strlen("Reason for kicking: "), 0);
				while (i < splitmsg.size())
				{
					send(it_s->_fd, (splitmsg.at(i)).c_str(), strlen((splitmsg.at(i)).c_str()), 0);
					send(it_s->_fd, " ", strlen(" "), 0);
					i++;
				}
				send(it_s->_fd, "\n", strlen("\n"), 0);
				this->clients.erase(it_s);
				for (it_o = this->operators.begin(); it_o != this->operators.end(); ++it_o)
					if (it_o->nickName == client_kick)
						this->operators.erase(it_o);
				return ;
			}
		}
	}
	if (it_s == this->clients.end())
		sendErrorMessage(client._fd, (client_kick + NO_CLIENT_M), ERR_NOSUCHNICK);
}

void Channel::exec_mode(std::string mode, Client &client, std::string arg)
{
	if (mode[1] == 'k')
	{
		if (mode[0] == '+')
		{
			if (arg == "")
				sendErrorMessage(client._fd, "Key for Channel not provided\n", TOO_MANY_ARGS);
			else
				this->pass = arg;
		}
		else
			this->pass = "";
	}
	else if (mode[1] == 'o')
	{
		std::vector<Client>::iterator it_s;
		for(it_s = this->clients.begin(); it_s != this->clients.end(); ++it_s)
		{
			if (it_s->nickName == arg)
				break ;
		}
		if (it_s != this->clients.end())
		{
			if (mode[0] == '+')
			{
					if (this->isOperator(*it_s))
						send(client._fd, "client is already an Operator\n", strlen("client is already an Operator\n"), 0);
					else
					{
						this->operators.push_back(*it_s);
						send(it_s->_fd, ("You are an operator of " + this->name + " channel \n").c_str(), strlen(("You are an operator of " + this->name + " channel \n").c_str()), 0);
					}
			}
			else
			{
				it_s = this->op_in_chan(it_s->_fd);
				if (it_s != this->operators.end())
				{
					if (it_s->nickName != client.nickName)
					{
						send(it_s->_fd, ("You are no longer an operator of " + this->name + " channel \n").c_str(), strlen(("You are no longer an operator of " + this->name + " channel \n").c_str()), 0);
						this->operators.erase(it_s);
					}
					else
						send(client._fd, "Cannot remove yourself from operators\n", strlen("Cannot remove yourself from operators\n"), 0);
				}
				else
					send(client._fd, "client is not an Operator\n", strlen("client is not an Operator\n"), 0);
			}
		}
		else
			sendErrorMessage(client._fd, (arg + NO_CLIENT_M), ERR_NOSUCHNICK);
	}
	else if (mode[1] == 'l')
	{
		if (mode[0] == '+')
		{
			if (std::atoi(arg.c_str()) <= 0)
				send(client._fd, "Invalid Limit\n", strlen("Invalid Limit\n"), 0);
			else
				this->client_limit = std::atoi(arg.c_str());
		}
	}
	this->setMode(mode[1], mode[0]);
}

int Channel::client_len(void)
{
	int len = 0;

	for (it_u = clients.begin(); it_u != clients.end(); it_u++)
		len++;
	return (len);
}

int Channel::isInvited(Client client)
{
	std::vector<Client>::iterator it;
	for (it = this->invites.begin(); it != this->invites.end(); it++)
	{
		if (it->nickName == client.nickName)
			return (1);
	}
	return (0);
}

int Channel::isMode(char m)
{
	std::map<char, bool>::iterator it;
	for (it = this->mode.begin(); it != this->mode.end(); it++)
	{
		if (it->first == m)
		{
			if (it->second == true)
				return (1);
			else if (it->second == false)
				return (0);
		}
	}
	return (2);
}

int Channel::isOperator(Client client)
{
	std::vector<Client>::const_iterator it;
	for (it = this->operators.begin(); it != this->operators.end(); it++)
	{
		if (it->nickName == client.nickName)
			return (1);
	}
	return (0);
}

int Channel::isclient(Client client)
{
	std::vector<Client>::const_iterator it;
	for (it = this->clients.begin(); it != this->clients.end(); it++)
	{
		if (it->nickName == client.nickName)
			return (1);
	}
	return (0);
}

std::vector<Client>::iterator Channel::client_in_chan(int fd)
{
	for (this->it_u = this->clients.begin(); this->it_u != this->clients.end(); it_u++)
	{
		if (it_u->_fd == fd)
			return (it_u);
	}
	return (it_u);
}

std::vector<Client>::iterator Channel::op_in_chan(int fd)
{
	for (this->it_o = this->operators.begin(); this->it_o != this->operators.end(); it_o++)
	{
		if (this->it_o->_fd == fd)
			return (it_o);
	}
	return (it_o);
}

std::vector<Client>::iterator Channel::inv_in_chan(int fd)
{
	for (this->it_i = this->invites.begin(); this->it_i != this->invites.end(); it_i++)
	{
		if (this->it_i->_fd == fd)
			return (it_i);
	}
	return (it_i);
}