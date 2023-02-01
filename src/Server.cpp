#include "Server.hpp"

#include <fcntl.h>
#include <sys/socket.h>

Server::Server(string pass, string port)
:
_port(port),
_password(pass)
{
}

Server::~Server()
{
}

void	Server::initilize(void)
{
	//struct addrinfo hints; //includes netdb in socket.h (probably fucked somthing) -second selotion is to define the struct here
	_svrSk = socket(AF_INET, SOCK_STREAM, 0); //ip4,sock_stream, get the default protocol for sock_stream (tcp)
	if ()
}