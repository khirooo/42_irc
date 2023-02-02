#ifndef SERVER_HPP
#define SERVER_HPP


#include <map>
#include <vector>
#include <poll.h>
#include <string>
#include <cstdlib>

using namespace std;
class Client; //to remove

#define BUFFER_SIZE	1000

class Server
{
private:
	string					_port;
	string					_password;
	char					_buffer[BUFFER_SIZE];	// to store clients data the size does not really mater here 
	int						_svrSk;		// main server socket (for clients to connect)
	vector<struct pollfd>	_pfds;		// vector with all client-server pollfd struct (socket id, event, revents) + the main server socket (pos = 0)
	//map<int, Client&>	_clients;		// maps of all clients and their assosiated server-client 

public:
	Server(string pass, string port = "6667");
	~Server();

	void	initilize(void);
	void	start(void);
};

#endif