#ifndef SERVER_HPP
#define SERVER_HPP


#include <map>
#include <vector>
#include <poll.h>
#include <string>
#include <cstdlib>
#include <exception>

#include "Client.hpp"
#include "Message.hpp"
#include "Utils.hpp"

#define BUFFER_SIZE	1024
typedef	std::vector<Channel*>::iterator	channel_it;

enum Status
{
	ON,
	OFF,
	RESTART
};

class Server
{
private:
	std::string					_host;
	std::string					_port;
	std::string					_password;
	char						_buffer[BUFFER_SIZE];
	int							_svrSk;		// main server socket (for clients to connect)
	std::vector<struct pollfd>	_pfds;		// vector with all client-server pollfd struct (socket id, event, revents) + the main server socket (pos = 0)
	std::map<int, Client*>		_clients;		// maps of all clients and their assosiated server-client 
	std::vector<Channel*>		_channels;
	std::string					_sudo;

public:

	Status						state;
	Server(std::string pass, std::string port);
	~Server();

	void					initilize(void);
	void					start(void);
	void					accept_connection(void);
	void					handel_message(struct pollfd* pfds_arr, int i);
	void					handel_command(int client_socket, Message m);
	bool					nick_used(int id, std::string nick);
	Client*					get_client(std::string name);
	Channel*				get_channel(std::string name);
	void					close_connection(struct pollfd* pfds_arr, int i);
	void					chan_mode(Client* client, Message& m, std::string dist, std::string moclientde);
	void					user_mode(Client* client, std::string dist, std::string mode);
	void					clean_server(void);
	// ALL SERVER COMMANDS (i might move this shit to a new class Command)
	void	cmd_pass(Client* client, Message& m);
	void	cmd_user(Client* client, Message& m);
	void	cmd_nick(Client* client, Message& m);
	void	cmd_prvmsg(Client* client, Message& m);
	void	cmd_notice(Client* client, Message& m);
	void	cmd_join(Client* client, Message& m);
	void	cmd_part(Client* client, Message& m);
	void	cmd_names(Client* client, Message& m);
	void	cmd_mode(Client* client, Message& m);
	void	cmd_topic(Client *client, Message& m);
	void	cmd_kick(Client *client, Message& m);
	void	cmd_invite(Client *client, Message& m);
	void	cmd_oper(Client *client, Message& m);
	void	cmd_kill(Client *client, Message& m);
	void	cmd_die(Client *client, Message& m);
	void	cmd_restart(Client *client, Message& m);
	void	cmd_quit(Client *client, Message& m);
	void	cmd_quot(Client* clien, Channel* channel);
	void	send_to_fd(int fd);
};

#endif