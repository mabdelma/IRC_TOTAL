#include "Server.hpp"
#include "Command.hpp"

Client &Utils::find(int fd) {
	for(std::vector<Client>::iterator it = Server::_clients.begin(); it != Server::_clients.end(); ++it) {
        if (it->_fd == fd) {
			return *it;
        }
    }
	throw Server::ServerException("Utils::find: client not found");
}

void Utils::signalHandler(int signum) {

    std::cout << RED << "Interrupt signal (" << signum << ") received." << RESET << "\n";

    for(std::vector<int>::iterator it = Server::_fds.begin(); it != Server::_fds.end(); ++it) {
            close(*it);
    }
	shutdown(Server::serverSocket, SHUT_RDWR);
    close(Server::serverSocket);
	Server::_fds.clear();
	Server::_clients.clear();
	Server::_channels.clear();
    exit(signum);
}

void Utils::closeThis(Client &client)
{
	close(client._fd);
	std::vector<Client>::iterator it_u;
	std::vector<Client>::iterator it_o;
	std::vector<Client>::iterator it_i;

    Server::_fds.erase(std::find(Server::_fds.begin(), Server::_fds.end(), client._fd));	
    Server::_clients.erase(std::find(Server::_clients.begin(), Server::_clients.end(), client));
	for (std::vector<Channel>::iterator it = Server::_channels.begin(); it != Server::_channels.end(); it++)
	{
		it_u = it->client_in_chan(Server::sd);
		it_o = it->op_in_chan(Server::sd);
		if (it_u != it->clients.end())
			it->clients.erase(it_u);
		if (it_o != it->operators.end())
		{
			it->operators.erase(it_o);
			it_o = it->clients.begin();
			if (it_o != it->clients.end() && it->operators.size() == 0)
				it->operators.push_back(*it_o);
		}
		it_i = it->inv_in_chan(Server::sd);
		if (it_i != it->invites.end())
			it->invites.erase(it_i);
	}
}


std::string Utils::to_string(int value) {
    std::stringstream ss;
	ss << value;
	std::string s = ss.str();
	return s;
}
std::vector<std::string> Utils::split(const std::string str) {
	std::vector<std::string> vector;
	std::istringstream iss(str);
	std::string cmd;
	while (iss >> std::skipws >> cmd)
		vector.push_back(cmd);
	return vector;
}
std::string Utils::trim(std::string &str) {
    size_t left = str.find_first_not_of('\n');
    size_t right = str.find_last_not_of('\n');
    if (left == std::string::npos || right == std::string::npos)
        return "";
    return str.substr(left, right - left + 1);
}

void sendErrorMessage(int fd, const std::string& message, const std::string& key) {
    std::string errorMsg = key + " ERROR: " + message;
    send(fd, errorMsg.c_str(), strlen(errorMsg.c_str()), 0);
}

void handleJoinCommand(const std::vector<std::string>& splitmsg, Command& cmd, Client* client) {
    if (splitmsg.size() == 2) {
        cmd.join(splitmsg.at(1), "", *client);
    } else if (splitmsg.size() == 3){
		cmd.join(splitmsg.at(1), splitmsg.at(2), *client);
	}
	else {
		return ;
    }
}

void handleKickCommand(const std::vector<std::string>& splitmsg, Command& cmd, Client* client) {
    if (splitmsg.size() >= 3) {
        cmd.kick(splitmsg.at(1), splitmsg.at(2), splitmsg, *client);
    } else {
        sendErrorMessage(client->_fd, "KICK command requires 3 or 4 arguments\n", TOO_MANY_ARGS);
    }
}

void handlePrivMsgCommand(const std::vector<std::string>& splitmsg, Command& cmd, Client* client) {
	if (splitmsg.size() >= 3) {
		cmd.privmsg(splitmsg.at(1), splitmsg, *client);
	} else if (splitmsg.size() == 2) {

		sendErrorMessage(client->_fd, "PRIVMSG command requires atleast 3 arguments\n", PRIVMSG_EMPTY);
	} else if(splitmsg.size() == 1) {
		sendErrorMessage(client->_fd, "PRIVMSG command requires atleast 3 arguments\n", ERR_NOSUCHNICK);
	} else { 
		std::string S = ERR_NEEDMOREPARAMS;
		S.append(" :Not enough parameters\r\n");
		send(client->_fd, S.c_str(), strlen(S.c_str()), 0);
		return;
	}
}

void handleNoticeCommand(const std::vector<std::string>& splitmsg, Command& cmd, Client* client) {
	if (splitmsg.size() >= 3) {
		cmd.notice(splitmsg.at(1), splitmsg, *client); 
	}
}

void	handleInviteCommand(const std::vector<std::string>& splitmsg, Command& cmd, Client* client)
{
	if (splitmsg.size() == 3) {
		cmd.invite(splitmsg.at(1), splitmsg.at(2), *client);
	} else {
		sendErrorMessage(client->_fd, "INVITE command requires 3 arguments\n", TOO_MANY_ARGS);
	}
}

void	handleTopicCommand(const std::vector<std::string>& splitmsg, Command& cmd, Client* client)
{
	if (splitmsg.size() == 3) {
		cmd.topic(splitmsg.at(1), splitmsg.at(2), *client);
	}else if (splitmsg.size() == 2){ 
		cmd.topic(splitmsg.at(1), "", *client);
	}else {
		sendErrorMessage(client->_fd, "INVITE command requires 1 or 2 arguments\n", TOO_MANY_ARGS);
	}
}

void handleWhoisCommand(const std::vector<std::string>& splitmsg, Command& cmd, Client* client) {
	(void)cmd;
	if (splitmsg.size() == 2) {
		std::string nick = "name : " + splitmsg.at(1) + "\r\n";
		send(client->_fd, nick.c_str(), strlen(nick.c_str()), 0);
	} else {
		sendErrorMessage(client->_fd, "WHOIS command requires 2 arguments\n", TOO_MANY_ARGS);
	}
}

void handleModeCommand(const std::vector<std::string>& splitmsg, Command& cmd, Client* client)
{
	if (splitmsg.size() == 3){
		cmd.mode(splitmsg.at(1), splitmsg.at(2), *client, "");
	}
	else if(splitmsg.size() == 4){
		cmd.mode(splitmsg.at(1), splitmsg.at(2), *client, splitmsg.at(3));
	}
	else {
		sendErrorMessage(client->_fd, "MODE command requires 2 or 3 arguments\n", TOO_MANY_ARGS);
	}
		
}

void handlePartCommand(const std::vector<std::string>& splitmsg, Command& cmd, Client* client)
{
	if (splitmsg.size() == 2){
		cmd.part(splitmsg.at(1), *client);
	}
	else {
		sendErrorMessage(client->_fd, "PART command requires 1 argument\n", TOO_MANY_ARGS);
	}
}