#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <cstring>
#include "Command.hpp"
#include "Utils.hpp"

#define JOIN "JOIN"
#define KICK "KICK"
#define NOTICE "NOTICE"
#define PRIVMSG "PRIVMSG"
#define INVITE "INVITE"
#define TOPIC "TOPIC"
#define MODE "MODE"
#define PART "PART"
#define WHOIS "WHOIS"
#define PING "PING"
#define PONG "TOPIC"
#define UNKNOWN_CMD "421"
#define ERR_NICKCOLLISION "436"
#define WRONG_PASS_CODE "464"
#define PRIVMSG_EMPTY "412"
#define NICKNAME_IN_USE "433"
#define ERR_NOSUCHNICK "401"
#define ERR_NOSUCHCHANNEL "403"
#define ERR_CANNOTSENDTOCHAN "404"
#define TOO_MANY_ARGS "461"
#define ERR_NOTREGISTERED "451"
#define ERR_TOOMANYCHANNELS "405"
#define ERR_ALREADYREGISTRED "462"
#define ERR_UNKNOWNMODE "472"
#define ERR_CHANOPRIVSNEEDED "482"
#define ERR_INVITEONLYCHAN "473"
#define ERR_BADCHANNELKEY "475"
#define ERR_clientONCHANNEL "443"
#define ERR_UNKNOWNCOMMAND "421"
#define ERR_CHANNELISFULL "471"
#define ERR_NEEDMOREPARAMS "461"
#define ERR_INVALIDKEY "525"
#define ERR_NOTONCHANNEL "442"


class Client {
   public:
    Client(int fd, int id);
    ~Client();

    std::string input;

    int _fd;
    int _id;
	int pass_issue;
	int alr_reg;
    bool isAuth;
    bool isOperator;
	bool change_flag;
	bool is_registered;
    std::string nickName;
    std::string clientName;
	std::string pass;
	std::vector<std::string> _cmd;

    void execute(std::string cmd, Client *it);
    void clientErase(Client &client);
    bool	parse_cmd(std::string str);
    friend std::ostream& operator<<(std::ostream& os, const Client& client);
    bool operator==(const Client& other) const {
        return (this->_fd == other._fd);
    }
    void change_client(Client *client, std::vector<std::string> splitmsg);
	int	authorise(Client *client, std::string cmd);
	bool	client_options(Client *client, std::vector<std::string> splitmsg);
	void	client_cmds(Client *client, std::vector<std::string> splitmsg);

};

void sendErrorMessage(int fd, const std::string& message, const std::string& key);
void handleJoinCommand(const std::vector<std::string>& splitmsg, Command& cmd, Client* client);
void handleKickCommand(const std::vector<std::string>& splitmsg, Command& cmd, Client* client);
void handlePrivMsgCommand(const std::vector<std::string>& splitmsg, Command& cmd, Client* client);
void handleNoticeCommand(const std::vector<std::string>& splitmsg, Command& cmd, Client* client);
void handleInviteCommand(const std::vector<std::string>& splitmsg, Command& cmd, Client* client);
void handleWhoisCommand(const std::vector<std::string>& splitmsg, Command& cmd, Client* client);
void handleModeCommand(const std::vector<std::string>& splitmsg, Command& cmd, Client* client);
void handleTopicCommand(const std::vector<std::string>& splitmsg, Command& cmd, Client* client);
void handlePartCommand(const std::vector<std::string>& splitmsg, Command& cmd, Client* client);

#endif

