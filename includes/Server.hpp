#ifndef SERVER_HPP
#define SERVER_HPP


#include <map>
#include <vector>
#include <poll.h>
#include <string>
#include <cstdlib>
#include "Client.hpp"


class Client; //to remove

#define BUFFER_SIZE	1000

class Server
{
private:
	std::string					_port;
	std::string					_password;
	char					_buffer[BUFFER_SIZE];	// to store clients data the size does not really mater here 
	int						_svrSk;		// main server socket (for clients to connect)
	std::vector<struct pollfd>	_pfds;		// vector with all client-server pollfd struct (socket id, event, revents) + the main server socket (pos = 0)
	//map<int, Client&>	_clients;		// maps of all clients and their assosiated server-client 

public:
	Server(std::string pass, std::string port = "6667");
	~Server();

	void	initilize(void);
	void	start(void);
	void	accept_connection(void);
	void	handel_message(struct pollfd* pfds_arr, int i);
	void	handel_command(int client_socket, std::string message);
};

#endif