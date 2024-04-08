#include "Client.hpp"
#include "Server.hpp"
#include <csignal>
#include <stdio.h>
#include <csignal>
#include <csetjmp>
#include <signal.h>
#include <unistd.h>

bool server_off = false;

void handle_sigint(int sig, siginfo_t *info, void *context) {
    (void)info;
    (void)context;
    if(sig)
    {
        server_off = true;
        std::cerr << "\n Signal received. Server shutdown..." << std::endl;
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: port" << std::endl;
        exit(0);
    }

       struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handle_sigint;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        if(SIGINT)
        {
            perror("sigaction");
            return 1;
        }
    }

    try {
        Server server(argv[1], argv[2]);
        server.serving();
    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Caught unknown exception." << std::endl;
    }

    return 0;
}
