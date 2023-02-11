#ifndef SERVER_HPP
#define SERVER_HPP


#include <map>
#include <vector>
#include <poll.h>
#include <string>
#include <cstdlib>

#include "Client.hpp"
#include "Message.hpp"
#include "Utils.hpp"

#define BUFFER_SIZE	1000
typedef	std::vector<Channel*>::iterator	channel_it;

class Server
{
private:
	std::string					_host;
	std::string					_port;
	std::string					_password;
	char						_buffer[BUFFER_SIZE];	// to store clients data the size does not really mater here 
	int							_svrSk;		// main server socket (for clients to connect)
	std::vector<struct pollfd>	_pfds;		// vector with all client-server pollfd struct (socket id, event, revents) + the main server socket (pos = 0)
	std::map<int, Client*>		_clients;		// maps of all clients and their assosiated server-client 
	std::vector<Channel*>		_channels;
	std::string					_sudo;

public:
	Server(std::string pass, std::string port = "6667");
	~Server();

	void					initilize(void);
	void					start(void);
	void					accept_connection(void);
	void					handel_message(struct pollfd* pfds_arr, int i);
	void					handel_command(int client_socket, Message m);
	bool					check_msg(Message m);
	bool					nick_used(int id, std::string nick);
	Client*					get_client(std::string name);
	Channel*				get_channel(std::string name);
	void					close_connection(struct pollfd* pfds_arr, int i);
	void					send_to_client(Client* client, std::string reply) const;
	void					send_to_channel(Client* client, Channel* channel, std::string reply) const;
	// ALL SERVER COMMANDS (i may move this shit to a new class Command)
	std::string	cmd_pass(Client* client, Message& m);
	std::string	cmd_user(Client* client, Message& m);
	std::string	cmd_nick(Client* client, Message& m);
	std::string cmd_prvmsg(Client* client, Message& m);
	std::string	cmd_join(Client* client, Message& m);
	std::string	cmd_part(Client* client, Message& m);
};

#endif