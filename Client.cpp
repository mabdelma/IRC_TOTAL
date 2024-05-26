
#include "Server.hpp"
#include "Command.hpp"

Client::Client(int fd, int id) : _fd(fd), _id(id), isAuth(false), isOperator(false), nickName(""), clientName(""), _cmd(2)
{
	is_registered = false;
	change_flag = false;
	pass_issue = 0;
	alr_reg = 0;
	input = "";
}

Client::~Client() {}

void Client::clientErase(Client &client)
{
	for(std::vector<Client>::iterator it = Server::_clients.begin(); it != Server::_clients.end(); ++it)
	{
		if (it->_fd == client._fd) {
			Server::_clients.erase(it);
			--it;
		}
	}
}

int Client::authorise(Client *client, std::string cmd)
{
	if(client->isAuth == true)
		return 2;
	if (parse_cmd(cmd))
	{
		if(!is_registered)
		{
			for(std::vector<Client>::iterator it = Server::_clients.begin(); it != Server::_clients.end(); ++it)
			{
				if(_cmd[0] != "" && _cmd[1] != "" && pass != ""){
					if (_cmd.size() > 1 && it->is_registered  && (it->nickName == _cmd[1]|| it->clientName == _cmd[0]) && !isAuth)
					{
						std::string S = ERR_ALREADYREGISTRED;
						S.append(" client already registered\n");
						send(client->_fd, S.c_str(), strlen(S.c_str()), 0);
						alr_reg = 1;
						return 0;
					}
				}
			}
		}
		client->nickName = _cmd[1];
		client->clientName = _cmd[0];
		if(client->nickName != "" && client->clientName != "" && pass == Server::getPassword() && !is_registered)
		{
			std::string wlcmMsg = ":irc 001 " + client->nickName + " :Welcome to FT_IRC, " + client->clientName + "@" + Server::_hostName + "\n"
								  ":irc 002 " + client->nickName + " :Host is " + Server::_hostName + ", running version 2.3\n"
								  ":irc 003 " + client->nickName + " :Created in the paradise in April 2024\n";
			send(client->_fd, wlcmMsg.c_str(), strlen(wlcmMsg.c_str()), 0);
			if(pass == Server::getPassword()){
				client->isAuth = true;
				is_registered = true;
			}
		}

		if(pass != "")
		{
			if(this->pass != Server::getPassword())
				{
					std::string S = WRONG_PASS_CODE;
					S.append(" : Password incorrect");
					send(this->_fd, S.c_str(), strlen(S.c_str()), 0);
					pass_issue = 1;
					return false;
				}
		}
		change_flag	= true;
		return 1;
	}
	return 0;
}

bool	Client::client_options(Client *client, std::vector<std::string> splitmsg)
{
	if (splitmsg.size() > 0 && (splitmsg.at(0) == "quit"))
	{
		Utils::closeThis(*client);
		return false;
	}	
	else if (splitmsg.size() > 0 && splitmsg.at(0) == "PASS") {
			 if (client->isAuth == true) {
				send(client->_fd, "462 :You may not reregister\r\n", 30, 0);
    	}
	}
	return true;
}

void Client::client_cmds(Client* client, std::vector<std::string> splitmsg) {
    if (splitmsg.empty()) {
        return;
    }

    Command cmd;
    std::string cmdType = splitmsg.at(0);
    if (cmdType == JOIN) {
        handleJoinCommand(splitmsg, cmd, client);
    } else if (cmdType == KICK) {
        handleKickCommand(splitmsg, cmd, client);
    } else if (cmdType == TOPIC) {
        handleTopicCommand(splitmsg, cmd, client);
    } else if (cmdType == PRIVMSG) {
        handlePrivMsgCommand(splitmsg, cmd, client);
    }else if (cmdType == NOTICE) {
        handleNoticeCommand(splitmsg, cmd, client);
    } else if (cmdType == INVITE) {
        handleInviteCommand(splitmsg, cmd, client);
    } else if (cmdType == PING) {
		send(client->_fd, "PONG\n", strlen("PONG\n"), 0);
	} else if (cmdType == WHOIS) {
		handleWhoisCommand(splitmsg, cmd, client);
	}else if (cmdType == MODE) {
		handleModeCommand(splitmsg, cmd, client);
	}else if (cmdType == PART){
		handlePartCommand(splitmsg, cmd, client);
	} else if (cmdType != "NICK" && cmdType != "PASS" && cmdType != "client" && cmdType != "CAP"){
		sendErrorMessage(client->_fd, "Unknown command\n", UNKNOWN_CMD);
	}
}

void Client::change_client(Client *client, std::vector<std::string> splitmsg)
{
	std::string old_name;
	if(!this->isAuth)
		return ;

	 if (splitmsg.size() < 2) {
		std::string S = "431";
		S.append(" :No nickname given\r\n");
        send(this->_fd, S.c_str(), strlen(S.c_str()), 0);
        return;
    }
	if (splitmsg.size() == 2 && splitmsg.at(0) == "NICK")
	{
		for(std::vector<Client>::iterator it = Server::_clients.begin(); it != Server::_clients.end(); ++it)
		{
			if (it->nickName == splitmsg.at(1))
			{
				std::string S = NICKNAME_IN_USE;
				S.append(" : Nickname is already in use\n");
				send(client->_fd, S.c_str(), strlen(S.c_str()), 0);
				return ;
			}
		}
		old_name = client->nickName;
		client->nickName = splitmsg.at(1);
		for (std::vector<Channel>::iterator it = Server::_channels.begin(); it != Server::_channels.end(); it++)
		{
			for (std::vector<Client>::iterator it_u = it->clients.begin(); it_u != it->clients.end(); it_u++)
				if (it_u->nickName == old_name)
					it_u->nickName = splitmsg.at(1);
			for (std::vector<Client>::iterator it_u = it->operators.begin(); it_u != it->operators.end(); it_u++)
				if (it_u->nickName == old_name)
					it_u->nickName = splitmsg.at(1);
			for (std::vector<Client>::iterator it_i = it->invites.begin(); it_i != it->invites.end(); it_i++)
				if (it_i->nickName == old_name)
					it_i->nickName = splitmsg.at(1);
		}
	}
	else
	{
		std::string S = ERR_NEEDMOREPARAMS;
		S.append(" :Not enough parameters\r\n");
		send(this->_fd, S.c_str(), strlen(S.c_str()), 0);
		return;
	}
}

void Client::execute(std::string cmd, Client *client)
{
	std::vector<std::string> splitmsg = Utils::split(cmd);

	if(!isAuth) {
		int client_flag = 0, nick_flag = 0, pass_flag = 0;
		for(size_t i = 0; splitmsg.size() > 0 && i < splitmsg.size(); i++)
		{
			if(splitmsg.at(i) == "client" && client_flag == 1)
			{
				sendErrorMessage(client->_fd, "Unknown command\n", UNKNOWN_CMD);
				return ;
			}
			if(splitmsg.at(i) == "NICK" && nick_flag == 1)
			{
				sendErrorMessage(client->_fd, "Unknown command\n", UNKNOWN_CMD);
				return ;
			}
			if(splitmsg.at(i) == "PASS" && pass_flag == 1)
			{
				sendErrorMessage(client->_fd, "Unknown command\n", UNKNOWN_CMD);
				return ;
			}
			if(splitmsg.at(i) == "client")
				client_flag = 1;
			if(splitmsg.at(i) == "NICK")
				nick_flag = 1;
			if(splitmsg.at(i) == "PASS")
				pass_flag = 1;
		}
	}

	if (!client_options(client, splitmsg))
		return ;
	if(!authorise(client, cmd))
	{
		if(splitmsg.size() > 0 && splitmsg.at(0) != "CAP"){
			if(pass_issue != 1 && alr_reg != 1)
			{
				std::string S = ERR_NOTREGISTERED;
				S.append(" You have not registered\n");
				send(client->_fd, S.c_str(), strlen(S.c_str()), 0);
			}
			else
			{
				Utils::closeThis(*client);
				return ;
			}
		}
	}

	if(splitmsg.size() > 0 && splitmsg.at(0) == "NICK" && change_flag == false)
		change_client(client, splitmsg);

	if ((splitmsg.size() > 1 && splitmsg.at(0) == "CAP"))
	{
		if(splitmsg.size() >= 3 && splitmsg.at(1) == "LS" && splitmsg.at(2) == "302")
		{
			std::string S = "CAP * ACK :multi-prefix\r\n";
			send(client->_fd, S.c_str(), strlen(S.c_str()), 0);
		}
		else if (splitmsg.size() >= 2 && splitmsg.at(1) == "LS")
		{
			std::string S = "CAP * ACK :multi-prefix\r\n";
			send(client->_fd, S.c_str(), strlen(S.c_str()), 0);
		}
		else if (splitmsg.size() >= 3 && splitmsg.at(1) == "REQ" && splitmsg.at(2) == "multi-prefix")
		{
			std::string S = "CAP * ACK :multi-prefix\n";
			send(client->_fd, S.c_str(), strlen(S.c_str()), 0);
		}
	}

	if(this->isAuth)
	{
		client_cmds(client, splitmsg);
		change_flag = false;
	}

	return ;
}

bool	Client::parse_cmd(std::string str)
{
	std::vector<std::string> vector =  Utils::split(str);

	for(std::vector<std::string>::iterator it = vector.begin(); it != vector.end();)
	{
		if(*it == "USER")
		{
			if(*it == vector.back())
				return false;
			else
				_cmd[0] = *(++it);
		}
		if(*it == "NICK")
		{
			if(*it == vector.back())
				return false;
			else
				_cmd[1] = *(++it);
		}
		if(*it == "PASS")
		{
			if(*it == vector.back())
				return false;
			else
				pass = *(++it);
			if (*(it) != vector.back())
			{
				if(*(it + 1) != "NICK" && *(it + 1) != "client")
				{
					std::string S = WRONG_PASS_CODE;
					S.append(" : Password incorrect");
					send(this->_fd, S.c_str(), strlen(S.c_str()), 0);
					pass_issue = 1;
					return false;
				}
			}
		}
		else
			++it;
	}

	return true;
}