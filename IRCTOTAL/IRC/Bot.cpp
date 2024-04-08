#include "Bot.hpp"

Bot::Bot(Channel *channel)
{
    (void)channel;
}
Bot::~Bot()
{
}

void Bot::ft_bot(int fd)
{
    std::string txt = "PRIVMSG #" + _channel->getName() + " :I'm here, I'm useless but I do it every 5 minutes \r\n";
    while (true) {
            send( fd, txt.c_str(), txt.size(), 0);
        sleep(300);
    }
}