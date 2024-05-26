#include "Server.hpp"


void Server::openSocket() {
    if ( (Server::serverSocket = socket( AF_INET, SOCK_STREAM, 0) ) == 0 ) {
        throw ServerException( "Failed to create socket" );
    }

    int opt = 1;
    if ( setsockopt(Server::serverSocket, SOL_SOCKET, SO_REUSEADDR, ( char * )&opt, sizeof( opt )) < 0 ) {
        throw ServerException( "setsockopt failed" );
    }

    Server::address.sin_family = AF_INET;
    Server::address.sin_addr.s_addr = INADDR_ANY;
    Server::address.sin_port = htons(_port);

    if ( bind(Server::serverSocket, ( struct sockaddr * )&Server::address, sizeof( Server::address )) < 0 ) {
        throw ServerException( "Bind failed" );
    }

    if ( listen(Server::serverSocket, MAX_CLIENTS) < 0 ) {
        throw ServerException( "Listen failed" );
    }

    addrlen = sizeof( Server::address );

    gethostname( c_hostName, MAX_HOST_NAME );
    Server::_hostName = c_hostName;
    std::cout << GREEN_LIGHT << "IRC Server started on port " << Server::_port << " : " << _hostName << std::endl;
    std::cout << "Waiting for incoming connections..." << RESET << std::endl;
}

void Server::run( void ) {

    int i = 0;
    
    for ( ;; ) {
        FD_ZERO( &Server::readfds );
        FD_SET( Server::serverSocket, &Server::readfds );
        Server::max_sd = serverSocket;

        for ( i = 0; i < static_cast<int>(Server::_fds.size()); i++ ) {
        
            Server::sd = Server::_fds.at(i);
            if ( Server::sd >= MAX_CLIENTS - 1 ) {
                for(std::vector<int>::iterator it = Server::_fds.begin(); it != Server::_fds.end(); ++it) {
                        close(*it);
                }
                shutdown(Server::serverSocket, SHUT_RDWR); 
                close(Server::serverSocket);
                Server::_fds.clear();
                Server::_clients.clear();
                Server::_channels.clear();
                throw ServerException( "Max clients reached" );
            }
            Server::_fds.at(i) > 0 ? 
                FD_SET( Server::sd, &Server::readfds ) : 
            ( void )0;

            if ( Server::sd > Server::max_sd )
                Server::max_sd = Server::sd;
        }

        int activity = select( Server::max_sd + 1, &Server::readfds, NULL, NULL, NULL );
        if ( ( activity < 0 ) && ( errno != EINTR ) ) {
            throw ServerException( "Select error" );
        }

        if ( FD_ISSET( Server::serverSocket, &Server::readfds ) ) {
            acceptConnection();
        }
        handleClientMessages();
    }
}


void Server::acceptConnection() {
    if ( ( Server::newSocket = accept( Server::serverSocket, ( struct sockaddr * )&Server::address, ( socklen_t * )&Server::addrlen ) ) < 0 ) { 
        throw ServerException( "Accept failed" );
    }

    Server::_fds.push_back( Server::newSocket );
    Server::_clients.push_back( Client( Server::newSocket, Server::newSocket - serverSocket ) );
    std::cout << GREEN << "New connection, " << "IP is : " << inet_ntoa(Server::address.sin_addr) << 
        ", port : " << Server::_port << RESET << std::endl;

    if ( fcntl( Server::newSocket, F_SETFL, O_NONBLOCK ) < 0 ) {
        throw ServerException( "Failed to set client socket to non-blocking mode" );
    }
}

void Server::handleClientMessages() {

    int i = 0;
	std::vector<Client>::iterator it_u;
	std::vector<Client>::iterator it_o;
	std::vector<Client>::iterator it_i;
    for ( i = 0; i < static_cast<int>( Server::_fds.size() ); i++ ) {
        Server::sd = Server::_fds.at(i);

        if ( FD_ISSET(Server::sd, &Server::readfds) ) {
		
            if ( (Server::valread = recv(Server::sd, Server::c_buffer, BUFFER_SIZE, 0)) <= 0 ) {

                std::cout << RED << "Host disconnected, IP " << inet_ntoa(Server::address.sin_addr) <<
                     ", port " << Server::_port << RESET << std::endl;
                FD_CLR(Server::sd, &Server::readfds);
                close(Server::sd);

                Server::_fds.erase(std::find(Server::_fds.begin(), Server::_fds.end(), Server::sd));
                Server::_clients.erase(std::find(Server::_clients.begin(), Server::_clients.end(), Utils::find(Server::sd)));
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

            } else {
                Server::valread < BUFFER_SIZE ? Server::c_buffer[Server::valread] = '\0' : Server::c_buffer[BUFFER_SIZE - 1] = '\0';
                for( std::vector<Client>::iterator it = Server::_clients.begin(); it != Server::_clients.end(); ++it ) {
                    if ( it->_fd == Server::sd ) {
                        it->input += Server::c_buffer;
                        std::string clientInput( Server::c_buffer );
                        curIndex = i;
                        if ( !clientInput.empty() ) {
                            it->execute( clientInput, &( *it ) );
                            break ; 
                        }
                        return ;
                    }
                }
            }
        }
    }
}

std::string Server::getPassword(void) {
    return Server::_password;
}

std::string Server::_password = "";
std::string Server::bufferStr = "";
std::string Server::_hostName = "";
char Server::c_buffer[BUFFER_SIZE]= {0};
char Server::c_hostName[MAX_HOST_NAME] = {0};
int Server::serverSocket = -1;
int Server::max_sd = -1;
int Server::sd = -1;
int Server::valread = -1;
int Server::_port = -1;
int Server::newSocket = -1;
int Server::curIndex = -1;
int Server::addrlen = sizeof(struct sockaddr_in);
std::vector<int> Server::_fds;
std::vector<Client> Server::_clients;
std::vector<Channel> Server::_channels;
struct sockaddr_in Server::address;
fd_set Server::readfds;