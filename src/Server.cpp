#include "Server.hpp"


#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <fcntl.h>
#include <list>


Server::Server(std::string pass, std::string port)
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
	int	exit_code;
	int	yes = 1;
	struct addrinfo hints, *servAddr, *p;
	char	name[1000];
	
	if (gethostname(name, sizeof name) == -1)
	{
		std::cerr << "Error: gethostname " <<  std::strerror(errno) << std::endl;
		exit (1);
	}
	_host= std::string(name);
	std::cout << "HOST: " << _host<< std::endl;
	//-- filling some hints for getaddrinfo so that it generates a addinfo struct for me
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; //using both ip4 and ip6
	hints.ai_socktype = SOCK_STREAM; //socket type
	hints.ai_flags = AI_PASSIVE; //fill my ip for me (if i pass NULL as node for getaddrinfo)

	exit_code = getaddrinfo(NULL, _port.c_str(), &hints, &servAddr);
	if (exit_code == -1)
	{
		std::cerr << "Error: getaddrinfo " << gai_strerror(exit_code) << std::endl;
		exit(1); //!!!! handel ur shit here
	}

	//-- looping throw the lists of addinfos until a creation(a bind) succeed 
	for (p = servAddr; p != NULL; p = p->ai_next)
	{
		// create a socket
		_svrSk = socket(p->ai_family, p->ai_socktype, p->ai_protocol); //ip4,sock_stream, get the default protocol for sock_stream (tcp)
		if (_svrSk == -1)
		{
			std::cerr << "Error: socket " <<  std::strerror(errno) << std::endl;
			continue; //try next socket
		}

		// reuse the address if it's used 
		if (setsockopt(_svrSk, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
		{
        	std::cerr << "Error: setsockopt " << strerror(errno) << std::endl;
        	continue ;
		}

		//bind the socket
		exit_code = bind(_svrSk, p->ai_addr, p->ai_addrlen);
		if (exit_code)
		{
			std::cerr << "Error: bind " << strerror(errno) << std::endl;
			close(_svrSk);
			continue;
		}
		else
			std::cout << "binded the server socket " << _svrSk << " with the port " << _port << std::endl<< std::endl<< std::endl;
		break; //success
	}
	freeaddrinfo(servAddr);
	if (p == NULL)
	{
		std::cerr << "Error: not able to bind a socket ... tneket" << std::endl;
		exit (1); //!!!! handel ur shit here
	}
	//listen to the socket dumb fuck
	if (listen(_svrSk, 10) == -1)
	{
		std::cerr << "Error: listen " << std::strerror(errno) << std::endl;
		exit(1); //!!!! handel ur shit
	}
	//--adding server socket to pollfd array
	struct pollfd	pfd;
	pfd.fd = _svrSk;
	pfd.events = POLLIN;
	_pfds.push_back(pfd); //_pfds.insert(_pfds.begin(), pfd);
	//NOTE : u might need to add a pollfd for STDIN for handelling server commands
}

void	Server::start(void)
{
	int	exit_code;

	while (true)
	{
		int	size = _pfds.size();
		struct pollfd* pfds_arr = new struct pollfd[size];
		std::copy(_pfds.begin(), _pfds.end(), pfds_arr);
		//std::cout << "_____POLL____"<< size << "____" << std::endl ;
		exit_code = poll(pfds_arr, size, -1);//block indefinitely
		if (exit_code == -1)
		{
			std::cerr << "Error: poll " << strerror(errno) << std::endl;
			exit(1); //!!!! handel ur shit here
		}

		//loop throught the pfds and check the events NOTE: break at poll return (faster!)
		for(int i = 0; i < size; i++)
		{
			if (pfds_arr[i].revents & POLLIN) //we got an event here!
			{
				if (pfds_arr[i].fd == _svrSk) //it's a new conncetion SO EXCITTEDDDD!!
					accept_connection();
				else //it's a new message from a client
					handel_message(pfds_arr, i);
			}
		}
		delete[] pfds_arr;
	}
}

void	Server::accept_connection(void)
{
	int						new_sock;
	char					name[1000];
	struct sockaddr_storage	client_addr;
	socklen_t				client_len = sizeof(client_addr);

	new_sock = accept(_svrSk, (sockaddr*)&client_addr, &client_len); //welcome b*t*h
	if (new_sock == -1)
		std::cerr << "Error: accept " << std::strerror(errno) << std::endl;
	else
		std::cout << "-- new connection \n" << std::endl;
	if (new_sock != -1)
	{
		if (getnameinfo((struct sockaddr*)&client_addr, client_len, name, sizeof(name), 0, 0, NI_NAMEREQD))
			std::cerr << "Error resolving client host name" << std::endl;
	}
	struct pollfd pfd;
	pfd.fd = new_sock;
	pfd.events = POLLIN;
	_pfds.push_back(pfd);
	_clients.insert(std::map<int, Client*>::value_type(new_sock, new Client(new_sock, std::string(name))));
	//NOTE : u might need to update the array here ! if u want ur server to be fast
}

void	Server::handel_message(struct pollfd* pfds_arr, int i)
{
	int	exit_code;
	Message	m;

	while (!m.is_complete())
	{
		memset(_buffer, 0, BUFFER_SIZE);
		exit_code = recv(pfds_arr[i].fd, _buffer, BUFFER_SIZE, 0); //flags = MSG_DONTWAIT might be faster ...
		if (exit_code <= 0) //connection closed or fail
		{
			if (exit_code == 0)
				std::cout << "\n-- connection closed\n\n";
			else if (exit_code == -1)
				std::cerr << "Error: recv " << std::strerror(errno) << std::endl;
			close(pfds_arr[i].fd); //chuss
			_clients.erase(pfds_arr[i].fd);
			_pfds.erase(_pfds.begin() + i);

			break ;
		}
		m.append(_buffer);
	}
	//std::cout << "m after the loop |" << m.get_msg() << "|" << std::endl;
	//std::cout << "here" << std::endl;
	if (!m.get_msg().empty())
		handel_command(pfds_arr[i].fd, m);
}

void	Server::handel_command(int socket, Message m)
{
	std::cout << "<--" << m.get_msg() << "." << std::endl;
	if(!check_msg(m))
	{
		// rigel rabha ...
		return ;
	}
	std::string	reply;
	if (m.get_cmd() == "PING")
		reply = RPL_PONG(_host, _clients[socket]->get_host());
	if (m.get_cmd() == "NICK")
		reply = cmd_nick(_clients[socket], m);
	if (m.get_cmd() == "USER")
		reply = cmd_user(_clients[socket], m);
	if (m.get_cmd() == "PRIVMSG")
		reply = cmd_prvmsg(_clients[socket], m);
	if (!reply.empty() && send(socket, reply.c_str(), reply.size(), 0) == reply.size())
		std::cout << "-->" << reply << "." << std::endl;
	else if (!reply.empty())
		std::cout << "i need to send the rest" << std::endl;
}

bool	Server::check_msg(Message m)
{
	std::string	cmd_list[8] = {"PASS", "NICK", "USER", "JOIN", "PART", "PRIVMSG", "QUIT", "KICK"};

	if (m.get_cmd() == "CAP" || m.get_cmd() == "PING")
		return true;
	//check if command exist
	if (std::count(cmd_list, cmd_list + 8, m.get_cmd()) == 0)
	{
		std::cerr << "nik mok ila hadi '"<< m.get_cmd() << "' command" << std::endl;
		return false;
	}

	//this is just a test dont panic
	std::vector<std::string>	params = m.get_params();
	for(std::vector<std::string>::iterator it = params.begin(); it != params.end(); it++)
	{
		if (*it == "")
		{
			std::cerr << "nik mok ila hadi command" << std::endl;
			return false;
		}
	}
	return true;
}

bool	Server::nick_used(int id, std::string nick)
{
	std::map<int, Client*>::iterator	it;
	for(it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->get_nick() == nick && it->first != id)
			return true;
	}
	return false;
}

Client*	Server::get_client(std::string nick)
{
	std::map<int, Client*>::iterator it;
	for (it = _clients.begin(); it != _clients.end(); it++)
	{
		std::cout << "cl_nick: " << it->second->get_nick() << "| " << nick << "." << std::endl;
		if (it->second->get_nick() == nick)
			return it->second;
	}
	return NULL;
}


// _________________________COMMANDS____________________________

std::string	Server::cmd_pass(Client* client, Message& m)
{
	if (client->get_state() == LOGIN)
		return ERR_ALREADYLOGEDIN(client->get_nick());
	if (client->get_state() == REGISTERED)
		return ERR_ALREADYREGISTRED(client->get_nick());
	std::vector<std::string>	param = m.get_params();
	if (param.size() == 0)
		return ERR_NEEDMOREPARAMS(client->get_nick(), "PASS");
	if (param[0] != _password)
		return ERR_PASSWDMISMATCH(client->get_nick());
	client->set_state(LOGIN);
	return "";
}

std::string	Server::cmd_nick(Client* client, Message& m)
{
	std::vector<std::string>	param = m.get_params();
	if (param.size() == 0)
		return ERR_NONICKNAMEGIVEN(client->get_nick());

	// check if valide else ERR_ERRONEUSNICKNAME
	// ERR_RESTRICTED donno yet
	/*	
		ERR_UNAVAILRESOURCE : Returned by a server to a user trying to change nickname
        when the desired nickname is blocked by the nick delay
        mechanism. 
	*/
	if (nick_used(client->get_skFd(), param[0])) //create a database of nicknames and compare  
	{
		return ERR_NICKNAMEINUSE(_host, client->get_nick(), param[0]);
	}
	client->set_nick(param[0]);
	if (client->get_state() == REGISTERED)
		RPL_WELCOME(_host, client->get_nick(), client->get_user(), client->get_host());
	if (client->get_user() != "*")
	{
		client->set_state(REGISTERED);
		return RPL_WELCOME(_host, client->get_nick(), client->get_user(), client->get_host());
	}
	return "";
}

std::string	Server::cmd_user(Client* client, Message& m)
{

	//handel shit errors

	std::vector<std::string>	param = m.get_params();
	client->set_user(param[0]);
	if (param[1] != "*")
		client->set_mode(param[1]);
	client->set_real(*param.end());
	if (client->get_nick() != "*")
	{
		client->set_state(REGISTERED);
		return RPL_WELCOME(_host, client->get_nick(), client->get_user(), client->get_host());
	}
	return "";
}

std::string	Server::cmd_prvmsg(Client* client, Message& m)
{
	//error handel

	Client* 	recp = get_client(m.get_params()[0]);
	if (recp == NULL)
	{
		std::cout << "client not found" << std::endl;
		return ""; //handel error
	}

	std::string	msg = m.get_params()[1];
	std::cout << "msg: " << msg << "." << std::endl;
	std::string	reply;
	reply = RPL_PRIVMSG(client->get_nick(), client->get_user(), client->get_host(), recp->get_nick(), msg);
	if (send(recp->get_skFd(), reply.c_str(), reply.size(), 0) == reply.size())
		std::cout << "-->" << reply << "." << std::endl;
	else
		std::cout << "!!!i need to send the rest" << std::endl;
	return "";
}