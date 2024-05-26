
#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>

# define RED_LIGHT "\033[1;38:5:131m"
# define GREEN_OLIVE "\033[1;38:5:107m"
# define GREEN_LIGHT "\033[1;38:5:150m"
# define MUSTARD "\033[38:5:136m"
# define B_MUSTARD "\033[38:5:136m"
# define RESET "\033[0m"

# define OP_ERR_M "Permission Denied- You're not an operator of the channel.\n"
# define NO_CLIENT_M " :No such nickname\n"
# define NO_CHAN_M " :No such channel.\n"
# define MODE_ERR_M " :is unknown mode char to me.\n"
# define NOT_CHAN_USR " :Cannot send to channel.\n"
# define NO_KEY_M " :Cannot join channel (+k).\n"
# define NO_INV_M " :Cannot join channel (+i).\n"
# define YES_CLIENT_M " :is already on channel\n"
# define INVALID_CHAN "Error: Invalid Channel Name. Channel name starts with \'#\' or \'&\'\n"
# define INVALID_CHAN_NAME "Error: Invalid Channel Name. Channel should have more than one character\n"
# define CHAN_FULL "  :Cannot join channel (+l)\n"
# define NOT_CLIENT_M "  :You're not on that channel\n"

class Client;

class Channel{
	private:
		int client_limit;
		std::string pass;
		std::string topic;
		std::map<char, bool> mode;
		Channel(void); 
	public:
	std::string name;
	std::vector<Client> invites;
	std::vector<Client> operators;
	std::vector<Client> clients;
	std::string message;
	std::vector<Client>::iterator it_u;
	std::vector<Client>::iterator it_o;
	std::vector<Client>::iterator it_i;
	Channel(std::string str_n, std::string str_p);
	~Channel(void);


	int getclientLimit(void); 
	std::string getTopic(void);
	std::string getPass(void);
	std::map<char, bool> getMode(void);
	std::vector<Client> getclients(void);
	std::vector<Client> getOperators(void);
	std::string getName(void) const;


	void setclientLimit(int num); 
	void setTopic(std::string str);
	void setPass(std::string str);
	void setMode(char m, char sign);
	

	void addclient(Client new_client);
	void kickclient(std::string client_kick, const std::vector<std::string>& splitmsg, Client client);
	void exec_mode(std::string mode, Client &client, std::string arg);
	std::vector<Client>::iterator client_in_chan(int fd);
	std::vector<Client>::iterator op_in_chan(int fd);
	std::vector<Client>::iterator inv_in_chan(int fd);
	int client_len(void);

	int isInvited(Client client);
	int isMode(char m);
	int isOperator(Client client);
	int isclient(Client client);
	
};
# endif
