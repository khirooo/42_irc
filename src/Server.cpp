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
#include <algorithm>

Server::Server(std::string pass, std::string port)
:
_port(port),
_password(pass),
_sudo("sudo")
{
	//add admin_user 
	_clients.insert(std::map<int, Client*>::value_type(-1, new Client(-1, std::string("admin"))));
	Channel*	channel = new Channel("#general");
	channel->set_topic("Spain without s...");
	_channels.push_back(channel);
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
	std::vector<Message>	m_vec;
	Message					*m = NULL;
	std::vector<std::string>	vec;

	do
	{
		memset(_buffer, 0, BUFFER_SIZE);
		exit_code = recv(pfds_arr[i].fd, _buffer, BUFFER_SIZE, 0); //flags = MSG_DONTWAIT might be faster ...
		if (exit_code <= 0) //connection closed or fail
		{
			if (exit_code == 0)
				std::cout << "\n-- connection closed\n\n";
			else if (exit_code == -1)
				std::cerr << "Error: recv " << std::strerror(errno) << std::endl;
			close_connection(pfds_arr, i);
			break ;
		}
		vec = ft_split(_buffer, "\r\n");
		for(int j = 0; j < vec.size(); j++)
		{
			m = new Message();
			m->append(vec[j].c_str());
			m_vec.push_back(*m);
			if (m->is_complete())
			{
				delete m;
				m = NULL;
			}
		}
	}while (m);
	//std::cout << "m after the loop |" << m.get_msg() << "|" << std::endl;
	for (int j = 0; j < m_vec.size(); j++)
		handel_command(pfds_arr[i].fd, m_vec[j]);
}

void	Server::close_connection(struct pollfd* pfds_arr, int i)
{
	Client*	client = _clients[pfds_arr[i].fd];
	std::vector<Channel*>	channels = client->get_channels();
	for (int i = 0; i < channels.size(); i++)
	{
		std::string	reply = RPL_QUIT(client->get_nick(), client->get_user(), client->get_host());
		send_to_channel(client, channels[i], reply);
		if (channels[i]->is_oper(client))
			channels[i]->remove_oper(client);
		else
			channels[i]->remove_client(client);
	}
	close(pfds_arr[i].fd);
	_clients.erase(pfds_arr[i].fd);
	_pfds.erase(_pfds.begin() + i);
}

void	Server::handel_command(int socket, Message m)
{
	std::cout << "<--" << m.get_msg() << "." << std::endl;
	std::string	cmd = m.get_cmd();
	std::vector<std::string> params = m.get_params();
	if(!check_msg(m))
	{
		std::cout << "did not pass the check" << std::endl;
		// rigel rabha ...
		return ;
	}
	std::string	reply;
	if (_clients[socket]->get_state() == HANDSHAKE && cmd != "PASS")
		reply = ERR_NOTREGISTERED(_host);
	else if (cmd == "CAP" && params[0] != "END")
		reply = RPL_CAP(_host);
	else if (cmd == "PASS")
		reply = cmd_pass(_clients[socket], m);
	else if (cmd == "PING")
		reply = RPL_PING(_host, params[0]);
	else if (cmd == "NICK")
		reply = cmd_nick(_clients[socket], m);
	else if (cmd == "USER")
		reply = cmd_user(_clients[socket], m);
	else if (cmd == "PRIVMSG")
		reply = cmd_prvmsg(_clients[socket], m);
	else if (cmd == "NOTICE")
		reply = cmd_notice(_clients[socket], m);
	else if (cmd == "JOIN")
		reply = cmd_join(_clients[socket], m);
	else if (cmd == "PART")
		reply = cmd_part(_clients[socket], m);
	else if (cmd == "NAMES")
		reply = cmd_names(_clients[socket], m);
	else if (cmd == "MODE")
		reply = cmd_mode(_clients[socket], m);
	else if (cmd == "TOPIC")
		reply = cmd_topic(_clients[socket], m);
	else if (cmd == "KICK")
		reply = cmd_kick(_clients[socket], m);
	else if (cmd == "INVITE")
		reply = cmd_invite(_clients[socket], m);
	else if (cmd == "OPER")
		reply = cmd_oper(_clients[socket], m);
	if (!reply.empty() && send(socket, reply.c_str(), reply.size(), 0) == reply.size())
		std::cout << "-->" << reply << "." << std::endl;
	else if (!reply.empty())
		std::cout << "i need to send the rest" << std::endl;
}

bool	Server::check_msg(Message m)
{
	std::string	cmd_list[14] = {"OPER", "PASS", "NICK", "USER", "JOIN", "PART", "PRIVMSG", "NOTICE", "QUIT", "NAMES", "MODE", "TOPIC", "KICK", "INVITE"};

	if (m.get_cmd() == "CAP" || m.get_cmd() == "PING")
		return true;
	//check if command exist
	if (std::count(cmd_list, cmd_list + 14, m.get_cmd()) == 0)
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

Client*	Server::get_client(std::string name)
{
	std::map<int, Client*>::iterator it;
	for (it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->get_nick() == name)
			return it->second;
	}
	return NULL;
}

Channel*	Server::get_channel(std::string name)
{
	channel_it	it;
	for(it = _channels.begin(); it != _channels.end(); ++it)
	{
		if ((*it)->get_name() == name)
			return *it;
	}
	return NULL;
}

void		Server::send_to_client(Client* client, std::string reply) const
{
	if (send(client->get_skFd(), reply.c_str(), reply.size(), 0) == reply.size())
		std::cout << "-->" << reply << "." << std::endl;
	else
		std::cout << "!!!i need to send the rest" << std::endl;
}

void		Server::send_to_channel(Client* client, Channel* channel, std::string reply) const
{
	std::vector<Client*>	clients = channel->get_clients();
	for(int	i= 0; i < clients.size(); i++)
	{
		if (clients[i]->get_nick() == client->get_nick())
			continue;
		send_to_client(clients[i], reply);
	}
}

std::string	Server::check_mode_usr(std::string	mode) const
{
	std::string	modes = "i";
	std::string clean_mode;
	bool		found = false;
	for(int i = 0; i < mode.size(); i++)
	{
		if (mode[i] == '+' || mode[i] == '-')
			clean_mode.push_back(mode[i]);
		else if (modes.find(mode[i]) != std::string::npos && clean_mode.find(mode[i]) == std::string::npos)
		{
			clean_mode.push_back(mode[i]);
			found = true;
		}
	}
	if (found)
		return clean_mode;
	return "";
}

std::string	Server::check_mode_chan(std::string	mode, std::string target) const
{
	std::string	modes;
	if (target == "")
		modes = "tim";
	else
		modes = "o";
	std::string clean_mode;
	bool		found = false;
	for(int i = 0; i < mode.size(); i++)
	{
		if (mode[i] == '+' || mode[i] == '-')
			clean_mode.push_back(mode[i]);
		else if (modes.find(mode[i]) != std::string::npos && clean_mode.find(mode[i]) == std::string::npos)
		{
			clean_mode.push_back(mode[i]);
			found = true;
		}
	}
	if (found)
		return clean_mode;
	return "";
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
		return ERR_PASSWDMISMATCH(_host);
	client->set_state(LOGIN);
	return "";
}

std::string	Server::cmd_nick(Client* client, Message& m)
{

	// ERR_RESTRICTED donno yet
	/*	
		ERR_UNAVAILRESOURCE : Returned by a server to a user trying to change nickname
        when the desired nickname is blocked by the nick delay
        mechanism. 
	*/
	if (client->get_state() == HANDSHAKE)
	{
		return ERR_NOTREGISTERED(_host); //should be not logged in
	}
	std::vector<std::string>	param = m.get_params();
	if (param.size() == 0)
		return ERR_NONICKNAMEGIVEN(client->get_nick());
	if (client->get_nick() == param[0])
		return "";
	if (nick_used(client->get_skFd(), param[0]))
	{
		return ERR_NICKNAMEINUSE(_host, client->get_nick(), param[0]);
	}
	std::string old = client->get_nick();
	client->set_nick(param[0]);
	if (client->get_state() == REGISTERED)
	{
		std::string reply = RPL_NICKCHANGE(old, client->get_user(), client->get_host(), param[0]);
		std::vector<Channel*>	channels = client->get_channels();
		send_to_client(client, reply);
		for (int i = 0; i < channels.size(); i++)
			send_to_channel(client, channels[i], reply);
		return "";
	}
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
	client->set_real(*(param.end() - 1));
	if (client->get_nick() != "*")
	{
		client->set_state(REGISTERED);
		return RPL_WELCOME(_host, client->get_nick(), client->get_user(), client->get_host());
	}
	return "";
}

std::string	Server::cmd_prvmsg(Client* client, Message& m)
{
	std::string	dist = m.get_params()[0];
	std::string msg = m.get_params()[1];
	std::string	reply;
	if (dist[0] == '#')
	{
		Channel*	channel = get_channel(dist);
		if (channel == NULL)
		{
			reply = ERR_NOSUCHNICK(_host, client->get_nick(), dist);
			send_to_client(client, reply);
		}
		else if (!client->in_channel(dist))
		{
			reply = ERR_CANNOTSENDTOCHAN(_host, client->get_nick(), dist);
			send_to_client(client, reply);
		}
		else if ((channel->get_mode() & 0b001) && !channel->is_oper(client))
		{
			reply = ERR_CHANOPRIVSNEEDED(_host, client->get_nick(), dist);
			send_to_client(client, reply);
		}
		else
		{
			reply = RPL_PRIVMSG(client->get_nick(), client->get_user(), client->get_host(), dist, msg);
			send_to_channel(client, channel, reply);
		}
	}
	else
	{
		Client*	cl = get_client(dist);
		if (cl == NULL)
		{
			std::string	reply = ERR_NOTONCHANNEL(_host, client->get_nick(), dist);
			send_to_client(client, reply);
		}
		else
		{
			reply = RPL_PRIVMSG(client->get_nick(), client->get_user(), client->get_host(), cl->get_nick(), msg);
			send_to_client(cl, reply);
		}
	}
	return "";
}

std::string	Server::cmd_notice(Client* client, Message& m)
{
	std::string	dist = m.get_params()[0];
	std::string msg = m.get_params()[1];
	std::string	reply;
	if (dist[0] == '#')
	{
		Channel*	channel = get_channel(dist);
		if (channel && client->in_channel(dist) && (!(channel->get_mode() & 0b001) || channel->is_oper(client)))
		{
			reply = RPL_NOTICE(client->get_nick(), client->get_user(), client->get_host(), dist, msg);
			send_to_channel(client, channel, reply);
		}
	}
	else
	{
		Client*	cl = get_client(dist);
		if (cl)
		{
			reply = RPL_NOTICE(client->get_nick(), client->get_user(), client->get_host(), cl->get_nick(), msg);
			send_to_client(cl, reply);
		}
	}
	return "";
}

std::string	Server::cmd_join(Client* client, Message& m)
{
	std::string channel_name = m.get_params()[0];
	std::string	reply;
	if (channel_name[0] != '#')
		return ERR_NOSUCHCHANNEL(_host, client->get_nick(), channel_name);
	if (client->in_channel(channel_name))
		return "";
	Channel*	channel = get_channel(channel_name);
	if (channel == NULL)
	{
		channel = new Channel(channel_name);
		_channels.push_back(channel);
		channel->add_oper(client);
	}
	else if ((channel->get_mode() & 0b010) && !client->is_invited(channel_name))
	{
		reply = ERR_INVITEONLYCHAN(_host, client->get_nick(), channel_name);
		send_to_client(client, reply);
		return "";
	}
	else
		channel->add_client(client);
	client->add_channel(channel);
	reply = RPL_JOIN(client->get_nick(), client->get_user(), client->get_host(), channel_name);
	send_to_client(client, reply);
	send_to_channel(client, channel, reply);
	if (channel->get_topic() != "")
	{
		reply = RPL_TOPIC(_host, client->get_nick(), channel_name, channel->get_topic());
		send_to_client(client, reply);
	}
	reply = RPL_NAMREPLY(_host, client->get_nick(), channel_name) + channel->get_clients_nick(client, 0b01);
	send_to_client(client, reply);
	reply = RPL_ENDOFNAMES(_host, client->get_nick(), channel_name);
	send_to_client(client, reply);
	return "";
}

std::string	Server::cmd_part(Client* client, Message& m)
{
	std::string				ch_name = m.get_params()[0];
	Channel*				ch = get_channel(ch_name);
	std::string				reply = RPL_PART(client->get_nick(), client->get_user(), client->get_host(), ch_name);
	send_to_client(client, reply);
	send_to_channel(client, ch, reply);
	client->part_channel(ch);
	if (ch->is_oper(client))
		ch->remove_oper(client);
	else
		ch->remove_client(client);
	client->remove_invite(ch);
	return "";
}

std::string	Server::cmd_names(Client* client, Message& m)
{
	std::vector<std::string>	vec_channel = ft_split(m.get_params()[0].c_str(), ",");
	Channel*	channel = get_channel(vec_channel[0]);
	std::string reply;
	if (client->in_channel(channel->get_name()))
		reply = channel->get_clients_nick(client, 0b01);
	else
		reply = channel->get_clients_nick(client, 0b00);
	if (reply != "\r\n")
	{
		reply = RPL_NAMREPLY(_host, client->get_nick(), channel->get_name()) + reply;
		send_to_client(client, reply);
	}
	reply = RPL_ENDOFNAMES(_host, client->get_nick(), channel->get_name());
	send_to_client(client, reply);
	return "";
}

std::string	Server::cmd_mode(Client* client, Message& m)
{
	std::string	dist = m.get_params()[0];
	std::string mode;
	if ( m.get_params().size() > 1)
		mode = m.get_params()[1];
	if (dist[0] == '#')
		chan_mode(client, m, dist, mode);
	else
		user_mode(client, m, dist, mode);
	return "";
}

void	Server::chan_mode(Client* client, Message& m, std::string dist, std::string mode)
{
	Channel*	channel = get_channel(dist);
	std::string	reply;
	if (channel == NULL)
	{
		reply = ERR_NOSUCHNICK(_host, client->get_nick(), dist);
		send_to_client(client, reply);
		return ;
	}
	if (mode == "") //get mode
	{
		reply = RPL_MODECHANNEL(_host, client->get_nick(), channel->get_name(), channel->get_mode_str());
		send_to_client(client, reply);
		return ;
	}
	if (!channel->is_oper(client))
	{
		reply = ERR_CHANOPRIVSNEEDED(_host, client->get_nick(), dist);
		send_to_client(client, reply);
		return ;
	}
	std::string	clean_mode;
	std::string	target;
	if (m.get_params().size() > 2)
		target = m.get_params()[2];
	clean_mode = check_mode_chan(mode, target);
	if (clean_mode == "")
	{
		reply = ERR_UMODEUNKNOWNFLAGCH(_host, client->get_nick());
		send_to_client(client, reply);
		return ;
	}
	if (target != "")
	{
		Client* cl = get_client(target);
		channel->set_mode(clean_mode, cl);
		reply = RPL_MODECHAN(client->get_nick(), client->get_user(), client->get_host(), channel->get_name(), clean_mode, target);
		send_to_client(client, reply);
		send_to_channel(client, channel, reply);
	}
	else
	{
		channel->set_mode(clean_mode, NULL);
		reply = RPL_MODECHANNEL(_host, client->get_nick(), channel->get_name(), channel->get_mode_str());
		send_to_client(client, reply);
		send_to_channel(client, channel, reply);
	}
}

void	Server::user_mode(Client* client, Message& m, std::string dist, std::string mode)
{
	Client*	cl = get_client(dist);
	std::string	reply;
	if (cl == NULL)
	{
		std::string	reply = ERR_NOTONCHANNEL(_host, client->get_nick(), dist);
		send_to_client(client, reply);
	}
	else if (client->get_nick() != dist)
	{
		reply = ERR_USERSDONTMATCH(_host, client->get_nick());
		send_to_client(client, reply);
	}
	else if (mode == "")
	{
		reply = RPL_MODEUSER(_host, client->get_nick(), client->get_mode_str());
		send_to_client(cl, reply);
	}
	else
	{
		std::string	clean_mode;
		clean_mode = check_mode_usr(mode);
		if (clean_mode == "")
		{
			reply = ERR_UMODEUNKNOWNFLAUSR(_host, client->get_nick());
			send_to_client(client, reply);
		}
		else
		{
			client->set_mode(clean_mode);
			reply = RPL_MODEUSER(_host, client->get_nick(), client->get_mode_str());
			send_to_client(client, reply);
		}
	}
}


std::string	Server::cmd_topic(Client* client, Message& m)
{
	std::string topic;
	std::string	dist = m.get_params()[0];
	if ( m.get_params().size() > 1)
		topic = m.get_params()[1];
	Channel*	channel = get_channel(dist);
	std::string reply;
	if (channel == NULL)
	{
		reply = ERR_NOSUCHNICK(_host, client->get_nick(), dist);
		send_to_client(client, reply);
	}
	if (topic == "")
	{
		std::string chan_topic = channel->get_topic();
		if (chan_topic == "")
			reply = RPL_NOTOPIC(_host, client->get_nick(), channel->get_name());
		else
			reply = RPL_TOPIC(_host, client->get_nick(), channel->get_name(), channel->get_topic());
		send_to_client(client, reply);
		return "";
	}
	char mode = channel->get_mode();
	if (((mode & 0b001) || (mode & 0b100)) && !channel->is_oper(client))
	{
		reply = ERR_CHANOPRIVSNEEDED(_host, client->get_nick(), channel->get_name());
		send_to_client(client, reply);
	}
	else
	{
		channel->set_topic(topic);
		reply = RPL_TOPICCHANGE(client->get_nick(), client->get_user(), client->get_host(), channel->get_name(), topic);
		send_to_client(client, reply);
		send_to_channel(client, channel, reply);
	}
	return "";
}

std::string	Server::cmd_kick(Client* client, Message& m)
{
	std::string	chan_name = m.get_params()[0];
	std::string target = m.get_params()[1];
	std::string	reply;

	Channel* channel = get_channel(chan_name);
	if (!channel)
	{
		reply = ERR_NOSUCHNICK(_host, client->get_nick(), chan_name);
		send_to_client(client, reply);
		return "";
	}
	if (!client->in_channel(chan_name))
	{
		reply = ERR_NOTONCHANNEL(_host, client->get_nick(), chan_name);
		send_to_client(client, reply);
		return "";
	}
	Client*	target_cl = get_client(target);
	if (!target_cl)
	{
		reply = ERR_NOSUCHNICK(_host, client->get_nick(), target);
		send_to_client(client, reply);
		return "";
	}
	if (!channel->is_oper(client))
	{
		reply = ERR_CHANOPRIVSNEEDED(_host, client->get_nick(), chan_name);
		send_to_client(client, reply);
		return "";
	}
	if (!target_cl->in_channel(chan_name))
	{
		reply = ERR_USERNOTINCHANNEL(_host, client->get_nick(), target, chan_name);
		send_to_client(client, reply);
		return "";
	}
	reply = RPL_KICK(client->get_nick(), client->get_user(), client->get_host(), chan_name, target);
	send_to_client(client, reply);
	send_to_channel(client, channel, reply);
	target_cl->part_channel(channel);
	target_cl->remove_invite(channel);
	if (channel->is_oper(target_cl))
		channel->remove_oper(target_cl);
	else
		channel->remove_client(target_cl);
	return "";
}

std::string	Server::cmd_invite(Client* client, Message& m)
{
	std::string	target = m.get_params()[0];
	std::string chan_name = m.get_params()[1];
	std::string	reply;

	Client*	target_cl = get_client(target);
	if (!target_cl)
	{
		reply = ERR_NOSUCHNICK(_host, client->get_nick(), target);
		send_to_client(client, reply);
		return "";
	}
	Channel* channel = get_channel(chan_name);
	if (!channel)
	{
		reply = ERR_NOSUCHNICK(_host, client->get_nick(), chan_name);
		send_to_client(client, reply);
		return "";
	}
	if (!channel->is_oper(client))
	{
		reply = ERR_CHANOPRIVSNEEDED(_host, client->get_nick(), chan_name);
		send_to_client(client, reply);
		return "";
	}
	reply = RPL_INVITING(_host, client->get_nick(), target, chan_name);
	send_to_client(client, reply);
	reply = RPL_INVITE(client->get_nick(), client->get_user(), client->get_host(), target, chan_name);
	send_to_client(target_cl, reply);
	target_cl->add_invite(channel);
	return "";
}

std::string	Server::cmd_oper(Client* client, Message& m)
{
	std::string	target = m.get_params()[0];
	std::string pass = m.get_params()[1];
	std::string	reply;
	Client*	target_cl = get_client(target);
	if (!target_cl)
	{
		reply = ERR_NOSUCHNICK(_host, client->get_nick(), target);
		send_to_client(client, reply);
		return "";
	}
	if (pass != _sudo)
	{
		reply = ERR_PASSWDMISMATCH(_host);
		send_to_client(client, reply);
		return "";
	}
	target_cl->set_mode("+o");
	reply = RPL_YOUREOPER(_host);
	send_to_client(target_cl, reply);
	return "";
}