
#ifndef COMMAND_HPP
# define COMMAND_HPP

#include <iostream>
#include <string>
#include <vector>

class Client;
class Channel;

class Command{
	private:
		std::string message;
		std::vector<Channel>::iterator chan_it;
		std::vector<Client>::iterator client_it;
	public:
		Command(void);
		~Command(void);

		void join(std::string channel_s, std::string key_s, Client client);
		void kick(std::string channel, std::string client_kick, const std::vector<std::string>& splitmsg, Client client);
		void invite(std::string client, std::string channel, Client client_o);
		void topic(std::string channel, std::string topic, Client client);
		void privmsg(std::string reciever, const std::vector<std::string>& splitmsg, Client client);
		void mode(std::string channel, std::string mode, Client client, std::string arg);
		void part(std::string channel, Client client);
		void notice(std::string reciever, const std::vector<std::string>& splitmsg, Client client);
		std::vector<std::string> ft_split(std::string str, char delimiter);
		std::vector<Channel>::iterator chan_exist(std::string channel);
		std::vector<Client>::iterator client_exist(std::string nick);

};

#endif