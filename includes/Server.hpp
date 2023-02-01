#ifndef SERVER_HPP
#define SERVER_HPP

using namespace std;

#include <map>
#include <vector>
#include <poll.h>
#include <string>

class Client; //to remove

class Server
{
private:
	string					_port;
	string					_password;
	string					_buffer;	// to store clients data
	int						_svrSk;		// main server socket (for clients to connect)
	vector<struct pollfd>	_pfds;		// vector with all client-server pollfd struct (socket id, event, revents) + the main server socket (pos = 0)
	map<int, Client&>	_clients;		// maps of all clients and their assosiated server-client 

public:
	Server(string pass, string port = "6667");
	~Server();

	void	initilize(void);
};





#endif