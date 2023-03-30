#include "Server.hpp"

Server::Server(std::string pass, std::string port)
:
_port(port),
_password(pass),
_sudo("sudo")
{
	std::cout << "Constructing the server" << std::endl;
	state = RESTART;
}

Server::~Server()
{
	std::cout << "Destructing the server" << std::endl;
}

void		Server::initilize(void)
{
	std::cout << "init..." << std::endl;
	Channel* general = new Channel("#general");
	general->set_mode("+t-im", NULL);
	general->set_topic("General use purpose channel");
	_channels.push_back(general);

	int	exit_code;
	int	yes = 1;
	struct addrinfo hints, *servAddr, *p;
	char	name[1000];
	
	if (gethostname(name, sizeof name) == -1)
	{
		std::cerr << "Error: gethostname " <<  std::strerror(errno) << std::endl;
		exit (1);
	}
	_host = std::string(name);

	//-- filling some hints for getaddrinfo so that it generates a addinfo struct for me
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; //using both ip4 and ip6
	hints.ai_socktype = SOCK_STREAM; //socket type
	hints.ai_flags = AI_PASSIVE; //fill my ip for me (if i pass NULL as node for getaddrinfo)

	exit_code = getaddrinfo(NULL, _port.c_str(), &hints, &servAddr);
	if (exit_code == -1)
	{
		std::cerr << "Error: getaddrinfo " << gai_strerror(exit_code) << std::endl;
		exit(1);
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
		break;
	}
	fcntl(_svrSk, F_SETFL, O_NONBLOCK);
	freeaddrinfo(servAddr);
	if (p == NULL)
	{
		std::cerr << "Error: not able to bind a socket" << std::endl;
		exit (1);
	}
	//listen to the socket dumb fuck
	if (listen(_svrSk, 128) == -1)
	{
		std::cerr << "Error: listen " << std::strerror(errno) << std::endl;
		exit(1);
	}
	//--adding server socket to pollfd array
	struct pollfd	pfd;
	pfd.fd = _svrSk;
	pfd.events = POLLIN;
	_pfds.push_back(pfd);
}

void		Server::start(void)
{
	std::cout << std::endl << "---------------- SERVER: ON -------------" << std::endl;
	state = ON;
	while (state == ON)
	{
		int	exit_code = 0;
		int	size = _pfds.size();
		struct pollfd* pfds_arr = new struct pollfd[size];
		std::copy(_pfds.begin(), _pfds.end(), pfds_arr);

		exit_code = poll(pfds_arr, size, -1); //block indefinitely
		if (exit_code == -1)
		{
			std::cerr << "Error: poll " << strerror(errno) << std::endl;
			exit(1);
		}

		for(int i = 0; i < size; i++)
		{
			if (pfds_arr[i].revents & POLLIN)
			{
				exit_code--;
				if (pfds_arr[i].fd == _svrSk)
					accept_connection();
				else
					handel_message(pfds_arr, i);
			}
			if (pfds_arr[i].revents & POLLOUT && is_in(pfds_arr[i].fd , _pfds))
				send_to_fd(pfds_arr[i].fd);
			if (!exit_code)
				break;
		}
		delete[] pfds_arr;
	}

	if (state == OFF)
		std::cout << std::endl << "---------------- SERVER: OFF -------------" << std::endl;
	if (state == RESTART)
		std::cout << std::endl << "---------------- SERVER: REASTARTING... -------------" << std::endl;
}

void		Server::accept_connection(void)
{
	int						new_sock;
	char					name[1000];
	struct sockaddr_storage	client_addr;
	socklen_t				client_len = sizeof(client_addr);

	new_sock = accept(_svrSk, (sockaddr*)&client_addr, &client_len);
	if (new_sock == -1)
		std::cerr << "Error: accept " << std::strerror(errno) << std::endl;
	else
		std::cout << std::endl << "-new connection" << std::endl;

	if (new_sock != -1)
	{
		if (getnameinfo((struct sockaddr*)&client_addr, client_len, name, sizeof(name), 0, 0, NI_NAMEREQD) != 0)
			std::cerr << "Error: getnameinfo " << strerror(errno) << std::endl;
	}
	struct pollfd pfd;
	pfd.fd = new_sock;
	pfd.events = POLLIN | POLLOUT;
	_pfds.push_back(pfd);
	Client* client = new Client(new_sock, std::string(name));
	_clients.insert(std::make_pair(new_sock, client));
}

void		Server::handel_message(struct pollfd* pfds_arr, int i)
{
	int	exit_code;
	std::vector<Message>	m_vec;
	//Message					*m = NULL;
	std::vector<std::string>	vec;
	Client* client = _clients[pfds_arr[i].fd];

	memset(_buffer, 0, BUFFER_SIZE);
	exit_code = recv(pfds_arr[i].fd, _buffer, BUFFER_SIZE, MSG_DONTWAIT);
	if (exit_code <= 0) //connection closed or fail
	{
		if (exit_code == 0)
			std::cout << "\n-- connection closed\n\n";
		else if (exit_code == -1)
			std::cerr << "Error: recv " << std::strerror(errno) << std::endl;
		close_connection(pfds_arr, i);
		return ;
	}
	vec = ft_split(_buffer, "\r\n");
	for(int j = 0; j < (int)vec.size(); j++)
	{
		// m = new Message();
		client->get_msg_in().append(vec[j].c_str());
		if (client->get_msg_in().is_complete())
		{
			m_vec.push_back(client->get_msg_in());
			client->get_msg_in().clear();
		}
	}
	for (int j = 0; j < (int)m_vec.size(); j++)
		handel_command(pfds_arr[i].fd, m_vec[j]);
}

void		Server::close_connection(struct pollfd* pfds_arr, int i)
{
	Client*	client = _clients[pfds_arr[i].fd];
	std::vector<Channel*>	channels = client->get_channels();
	std::string	reply = RPL_QUIT(client->get_nick(), client->get_user(), client->get_host(), "QUIT :Client Quit");
	for (int i = 0; i < (int)channels.size(); i++)
	{
		append_to_channel(client, channels[i], reply);
		if (channels[i]->is_oper(client))
			channels[i]->remove_oper(client);
		else
			channels[i]->remove_client(client);
	}
	close(pfds_arr[i].fd);
	_clients.erase(pfds_arr[i].fd);
	pfds_arr[i].fd = -1;
	_pfds.erase(_pfds.begin() + i);
	delete client;
}

void		Server::handel_command(int socket, Message m)
{
	std::cout << "<--" << m.get_msg() << std::endl;
	
	Client* client = _clients[socket];
	std::string	cmd = m.get_cmd();
	std::vector<std::string> params = m.get_params();
	if(!check_msg(m))
	{
		client->append_msg_out(ERR_UNKNOWNCOMMAND(_host, client->get_nick(), cmd));
		return ;
	}
	if (cmd == "PASS")
		cmd_pass(client, m);
	else if (client->get_state() == HANDSHAKE)
		client->append_msg_out(ERR_NOTREGISTERED(_host));
	else if (cmd == "CAP" && params[0] != "END")
		client->append_msg_out(RPL_CAP(_host));
	else if (cmd == "NICK")
		cmd_nick(client, m);
	else if (cmd == "USER")
		cmd_user(client, m);
	else if (client->get_state() != REGISTERED)
		client->append_msg_out(ERR_NOTREGISTERED(_host));
	else if (cmd == "PING")
		client->append_msg_out(RPL_PING(_host, params[0]));
	else if (cmd == "PRIVMSG")
		cmd_prvmsg(client, m);
	else if (cmd == "NOTICE")
		cmd_notice(client, m);
	else if (cmd == "JOIN")
		cmd_join(client, m);
	else if (cmd == "PART")
		cmd_part(client, m);
	else if (cmd == "NAMES")
		cmd_names(client, m);
	else if (cmd == "MODE")
		cmd_mode(client, m);
	else if (cmd == "TOPIC")
		cmd_topic(client, m);
	else if (cmd == "KICK")
		cmd_kick(client, m);
	else if (cmd == "INVITE")
		cmd_invite(client, m);
	else if (cmd == "OPER")
		cmd_oper(client, m);
	else if (cmd == "KILL")
		cmd_kill(client, m);
	else if (cmd == "DIE")
		cmd_die(client, m);
	else if (cmd == "RESTART")
		cmd_restart(client, m);
	else if (cmd == "QUIT")
		cmd_quit(client, m);
}

bool		Server::nick_used(int id, std::string nick)
{
	std::map<int, Client*>::iterator	it;
	for(it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->get_nick() == nick && it->first != id)
			return true;
	}
	return false;
}

Client*		Server::get_client(std::string name)
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

// _________________________COMMANDS____________________________

void		Server::cmd_pass(Client* client, Message& m)
{
	std::string reply;
	if (client->get_state() == LOGIN)
	{
		client->append_msg_out(ERR_ALREADYLOGEDIN(client->get_nick()));
		return ;
	}
	else if (client->get_state() == REGISTERED)
	{
		client->append_msg_out(ERR_ALREADYREGISTRED(client->get_nick()));
		return ;
	}
	std::vector<std::string>	param = m.get_params();
	if (param.size() == 0)
	{
		client->append_msg_out(ERR_NEEDMOREPARAMS(client->get_nick(), "PASS"));
		return ;
	}
	if (param[0] != _password)
	{
		client->append_msg_out(ERR_PASSWDMISMATCH(_host));
		return ;
	}
	client->set_state(LOGIN);
}

void		Server::cmd_nick(Client* client, Message& m)
{
	std::vector<std::string>	param = m.get_params();
	if (param.size() == 0)
	{
		client->append_msg_out(ERR_NONICKNAMEGIVEN(client->get_nick()));
		return;
	}
	if (client->get_nick() == param[0])
		return ;
	if (!check_nick_name(param[0]))
	{
		client->append_msg_out(ERR_ERRONEUSNICKNAME(_host, client->get_nick(), param[0]));
		return;
	}
	if (nick_used(client->get_skFd(), param[0]))
	{
		client->append_msg_out(ERR_NICKNAMEINUSE(_host, client->get_nick(), param[0]));
		return;
	}
	std::string old = client->get_nick();
	client->set_nick(param[0]);
	if (client->get_state() == REGISTERED)
	{
		std::string reply = RPL_NICKCHANGE(old, client->get_user(), client->get_host(), param[0]);
		std::vector<Channel*>	channels = client->get_channels();
		client->append_msg_out(reply);
		for (int i = 0; i < (int)channels.size(); i++)
			append_to_channel(client, channels[i], reply);
		return ;
	}
	if (client->get_user() != "*")
	{
		client->set_state(REGISTERED);
		client->append_msg_out(RPL_WELCOME(_host, client->get_nick(), client->get_user(), client->get_host()));
	}
}

void		Server::cmd_user(Client* client, Message& m)
{
	if (client->get_state() == REGISTERED)
	{
		client->append_msg_out(ERR_ALREADYREGISTRED(client->get_nick()));
		return ;
	}
	std::vector<std::string>	param = m.get_params();
	if (param.size() < 4)
	{
		client->append_msg_out(ERR_NEEDMOREPARAMS(client->get_nick(), "USER"));
		return ;
	}
	client->set_user(param[0]);
	if (param[1] != "*")
		client->set_mode(param[1]);
	client->set_real(param[3]);
	if (client->get_nick() != "*")
	{
		client->set_state(REGISTERED);
		client->append_msg_out(RPL_WELCOME(_host, client->get_nick(), client->get_user(), client->get_host()));
	}
}

void		Server::cmd_prvmsg(Client* client, Message& m)
{
	std::vector<std::string>	param = m.get_params();
	if (param.size() < 2)
	{
		client->append_msg_out(ERR_NEEDMOREPARAMS(client->get_nick(), "PRIVMSG"));
		return ;
	}
	std::string	dist = m.get_params()[0];
	std::string msg = m.get_params()[1];
	if (dist[0] == '#')
	{
		Channel*	channel = get_channel(dist);
		if (msg == "!quot" && channel->get_mode() & 0b1000)
		{
			cmd_quot(client, channel);
			return ;
		}
		if (channel == NULL)
			client->append_msg_out(ERR_NOSUCHNICK(_host, client->get_nick(), dist));
		else if (!client->in_channel(dist))
			client->append_msg_out(ERR_CANNOTSENDTOCHAN(_host, client->get_nick(), dist));
		else if ((channel->get_mode() & 0b0001) && !channel->is_oper(client))
			client->append_msg_out(ERR_CHANOPRIVSNEEDED(_host, client->get_nick(), dist));
		else
			append_to_channel(client, channel, RPL_PRIVMSG(client->get_nick(), client->get_user(), client->get_host(), dist, msg));
	}
	else
	{
		Client*	cl = get_client(dist);
		if (cl == NULL)
			client->append_msg_out(ERR_NOTONCHANNEL(_host, client->get_nick(), dist));
		else
			cl->append_msg_out(RPL_PRIVMSG(client->get_nick(), client->get_user(), client->get_host(), cl->get_nick(), msg));
	}
}

void		Server::cmd_notice(Client* client, Message& m)
{
	std::vector<std::string>	param = m.get_params();
	if (param.size() < 2)
		return ;
	std::string	dist = m.get_params()[0];
	std::string msg = m.get_params()[1];
	if (dist[0] == '#')
	{
		Channel*	channel = get_channel(dist);
		if (channel && client->in_channel(dist) && (!(channel->get_mode() & 0b001) || channel->is_oper(client)))
			append_to_channel(client, channel, RPL_NOTICE(client->get_nick(), client->get_user(), client->get_host(), dist, msg));
	}
	else
	{
		Client*	cl = get_client(dist);
		if (cl)
			cl->append_msg_out(RPL_NOTICE(client->get_nick(), client->get_user(), client->get_host(), cl->get_nick(), msg));
	}
}

void		Server::cmd_join(Client* client, Message& m)
{
	std::vector<std::string>	param = m.get_params();
	if (param.size() < 1)
	{
		client->append_msg_out(ERR_NEEDMOREPARAMS(client->get_nick(), "JOIN"));
		return ;
	}
	std::string channel_name = m.get_params()[0];
	if (!check_chan_name(channel_name))
	{
		client->append_msg_out(ERR_NOSUCHCHANNEL(_host, client->get_nick(), channel_name));
		return ;
	}
	if (client->in_channel(channel_name))
		return ;
	Channel*	channel = get_channel(channel_name);
	if (channel == NULL)
	{
		channel = new Channel(channel_name);
		_channels.push_back(channel);
		channel->add_oper(client);
		client->add_channel(channel);
		std::string reply = RPL_JOIN(client->get_nick(), client->get_user(), client->get_host(), channel_name);
		client->append_msg_out(reply);
		client->append_msg_out(RPL_NAMREPLY(_host, client->get_nick(), channel_name) + channel->get_clients_nick(0b01));
		client->append_msg_out(RPL_ENDOFNAMES(_host, client->get_nick(), channel_name));
		return ;
	}
	if ((channel->get_mode() & 0b010) && !client->is_invited(channel_name))
	{
		client->append_msg_out(ERR_INVITEONLYCHAN(_host, client->get_nick(), channel_name));
		return ;
	}
	client->add_channel(channel);
	channel->add_client(client);
	std::string reply = RPL_JOIN(client->get_nick(), client->get_user(), client->get_host(), channel_name);
	client->append_msg_out(reply);
	append_to_channel(client, channel, reply);
	if (channel->get_topic() != "")
		client->append_msg_out(RPL_TOPIC(_host, client->get_nick(), channel_name, channel->get_topic()));
	client->append_msg_out(RPL_NAMREPLY(_host, client->get_nick(), channel_name) + channel->get_clients_nick(0b01));
	client->append_msg_out(RPL_ENDOFNAMES(_host, client->get_nick(), channel_name));
}

void		Server::cmd_part(Client* client, Message& m)
{
	std::vector<std::string>	param = m.get_params();
	if (param.size() < 1)
	{
		client->append_msg_out(ERR_NEEDMOREPARAMS(client->get_nick(), "PART"));
		return ;
	}
	std::string				ch_name = m.get_params()[0];
	Channel*				ch = get_channel(ch_name);
	if (!ch)
	{
		client->append_msg_out(ERR_NOSUCHCHANNEL(_host, client->get_nick(), ch_name));
		return ;
	}
	if (!client->in_channel(ch_name))
	{
		client->append_msg_out(ERR_NOTONCHANNEL(_host, client->get_nick(), ch_name));
		return ;
	}
	std::string				reply = RPL_PART(client->get_nick(), client->get_user(), client->get_host(), ch_name);
	client->append_msg_out(reply);
	append_to_channel(client, ch, reply);
	client->part_channel(ch);
	if (ch->is_oper(client))
		ch->remove_oper(client);
	else
		ch->remove_client(client);
	client->remove_invite(ch);
}

void		Server::cmd_names(Client* client, Message& m)
{
	std::vector<std::string>	param = m.get_params();
	if (param.size() < 1)
	{
		client->append_msg_out(ERR_NEEDMOREPARAMS(client->get_nick(), "NAMES"));
		return ;
	}
	std::vector<std::string>	vec_channel = ft_split(m.get_params()[0].c_str(), ",");
	if (vec_channel.size() < 1)
	{
		client->append_msg_out(ERR_NOSUCHCHANNEL(_host, client->get_nick(), m.get_params()[0]));
		return ;
	}
	Channel*	channel = get_channel(vec_channel[0]);
	if (!channel)
	{
		client->append_msg_out(ERR_NOSUCHCHANNEL(_host, client->get_nick(), vec_channel[0]));
		return ;
	}
	std::string reply;
	if (client->in_channel(channel->get_name()))
		reply = channel->get_clients_nick(0b01);
	else
		reply = channel->get_clients_nick(0b00);
	if (reply != "\r\n")
	{
		reply = RPL_NAMREPLY(_host, client->get_nick(), channel->get_name()) + reply;
		client->append_msg_out(reply);
	}
	client->append_msg_out(RPL_ENDOFNAMES(_host, client->get_nick(), channel->get_name()));
}

void		Server::cmd_mode(Client* client, Message& m)
{
	std::vector<std::string>	param = m.get_params();
	if (param.size() < 1)
	{
		client->append_msg_out(ERR_NEEDMOREPARAMS(client->get_nick(), "MODE"));
		return ;
	}
	std::string	dist = m.get_params()[0];
	std::string mode;
	if ( m.get_params().size() > 1)
		mode = m.get_params()[1];
	if (dist[0] == '#')
		chan_mode(client, m, dist, mode);
	else
		user_mode(client, dist, mode);
}

void		Server::chan_mode(Client* client, Message& m, std::string dist, std::string mode)
{
	Channel*	channel = get_channel(dist);
	std::string	reply;
	if (channel == NULL)
	{
		client->append_msg_out(ERR_NOSUCHNICK(_host, client->get_nick(), dist));
		return ;
	}
	if (mode == "") //get mode
	{
		client->append_msg_out(RPL_MODECHANNEL(_host, client->get_nick(), channel->get_name(), channel->get_mode_str()));
		return ;
	}
	if (!channel->is_oper(client))
	{
		client->append_msg_out(ERR_CHANOPRIVSNEEDED(_host, client->get_nick(), dist));
		return ;
	}
	std::string	clean_mode;
	std::string	target;
	if (m.get_params().size() > 2)
		target = m.get_params()[2];
	clean_mode = check_mode_chan(mode, target);
	if (clean_mode == "")
	{
		client->append_msg_out(ERR_UMODEUNKNOWNFLAGCH(_host, client->get_nick()));
		return ;
	}
	if (target != "")
	{
		Client* cl = get_client(target);
		channel->set_mode(clean_mode, cl);
		reply = RPL_MODECHAN(client->get_nick(), client->get_user(), client->get_host(), channel->get_name(), clean_mode, target);
		client->append_msg_out(reply);
		append_to_channel(client, channel, reply);
	}
	else
	{
		channel->set_mode(clean_mode, NULL);
		reply = RPL_MODECHANNEL(_host, client->get_nick(), channel->get_name(), channel->get_mode_str());
		client->append_msg_out(reply);
		append_to_channel(client, channel, reply);
	}
}

void		Server::user_mode(Client* client, std::string dist, std::string mode)
{
	Client*	cl = get_client(dist);
	std::string	reply;
	if (cl == NULL)
		client->append_msg_out(reply = ERR_NOTONCHANNEL(_host, client->get_nick(), dist));
	else if (client->get_nick() != dist)
		client->append_msg_out(ERR_USERSDONTMATCH(_host, client->get_nick()));
	else if (mode == "")
		cl->append_msg_out(RPL_MODEUSER(_host, client->get_nick(), client->get_mode_str()));
	else
	{
		std::string	clean_mode;
		clean_mode = check_mode_usr(mode);
		if (clean_mode == "")
			client->append_msg_out(ERR_UMODEUNKNOWNFLAUSR(_host, client->get_nick()));
		else
		{
			client->set_mode(clean_mode);
			client->append_msg_out(RPL_MODEUSER(_host, client->get_nick(), client->get_mode_str()));
		}
	}
}

void		Server::cmd_topic(Client* client, Message& m)
{
	std::vector<std::string>	param = m.get_params();
	std::string topic;
	if (param.size() < 1)
	{
		client->append_msg_out(ERR_NEEDMOREPARAMS(client->get_nick(), "TOPIC"));
		return ;
	}
	std::string	dist = m.get_params()[0];
	if ( m.get_params().size() > 1)
		topic = m.get_params()[1];
	Channel*	channel = get_channel(dist);
	std::string reply;
	if (channel == NULL)
	{
		client->append_msg_out(ERR_NOSUCHNICK(_host, client->get_nick(), dist));
		return ;
	}
	if (topic == "")
	{
		std::string chan_topic = channel->get_topic();
		if (chan_topic == "")
			client->append_msg_out(RPL_NOTOPIC(_host, client->get_nick(), channel->get_name()));
		else
			client->append_msg_out(RPL_TOPIC(_host, client->get_nick(), channel->get_name(), channel->get_topic()));
		return ;
	}
	char mode = channel->get_mode();
	if (((mode & 0b001) || (mode & 0b100)) && !channel->is_oper(client))
		client->append_msg_out(ERR_CHANOPRIVSNEEDED(_host, client->get_nick(), channel->get_name()));
	else
	{
		channel->set_topic(topic);
		reply = RPL_TOPICCHANGE(client->get_nick(), client->get_user(), client->get_host(), channel->get_name(), topic);
		client->append_msg_out(reply);
		append_to_channel(client, channel, reply);
	}
}

void		Server::cmd_kick(Client* client, Message& m)
{
	std::vector<std::string>	param = m.get_params();
	if (param.size() < 2)
	{
		client->append_msg_out(ERR_NEEDMOREPARAMS(client->get_nick(), "KICK"));
		return ;
	}
	std::string	chan_name = m.get_params()[0];
	std::string target = m.get_params()[1];
	std::string	reply;

	Channel* channel = get_channel(chan_name);
	if (!channel)
	{
		client->append_msg_out(ERR_NOSUCHNICK(_host, client->get_nick(), chan_name));
		return ;
	}
	if (!client->in_channel(chan_name))
	{
		client->append_msg_out(ERR_NOTONCHANNEL(_host, client->get_nick(), chan_name));
		return ;
	}
	Client*	target_cl = get_client(target);
	if (!target_cl)
	{
		client->append_msg_out(ERR_NOSUCHNICK(_host, client->get_nick(), target));
		return ;
	}
	if (!channel->is_oper(client))
	{
		client->append_msg_out( ERR_CHANOPRIVSNEEDED(_host, client->get_nick(), chan_name));
		return ;
	}
	if (!target_cl->in_channel(chan_name))
	{
		client->append_msg_out(ERR_USERNOTINCHANNEL(_host, client->get_nick(), target, chan_name));
		return ;
	}
	reply = RPL_KICK(client->get_nick(), client->get_user(), client->get_host(), chan_name, target);
	client->append_msg_out(reply);
	append_to_channel(client, channel, reply);
	target_cl->part_channel(channel);
	target_cl->remove_invite(channel);
	if (channel->is_oper(target_cl))
		channel->remove_oper(target_cl);
	else
		channel->remove_client(target_cl);
}

void		Server::cmd_invite(Client* client, Message& m)
{
	std::vector<std::string>	param = m.get_params();
	if (param.size() < 2)
	{
		client->append_msg_out(ERR_NEEDMOREPARAMS(client->get_nick(), "INVITE"));
		return ;
	}
	std::string	target = m.get_params()[0];
	std::string chan_name = m.get_params()[1];

	Client*	target_cl = get_client(target);
	if (!target_cl)
	{
		client->append_msg_out(ERR_NOSUCHNICK(_host, client->get_nick(), target));
		return ;
	}
	Channel* channel = get_channel(chan_name);
	if (!channel)
	{
		client->append_msg_out(ERR_NOSUCHNICK(_host, client->get_nick(), chan_name));
		return ;
	}
	if (!channel->is_oper(client))
	{
		client->append_msg_out(ERR_CHANOPRIVSNEEDED(_host, client->get_nick(), chan_name));
		return ;
	}
	client->append_msg_out(RPL_INVITING(_host, client->get_nick(), target, chan_name));
	target_cl->append_msg_out(RPL_INVITE(client->get_nick(), client->get_user(), client->get_host(), target, chan_name));
	target_cl->add_invite(channel);
}

void		Server::cmd_oper(Client* client, Message& m)
{
	std::vector<std::string>	param = m.get_params();
	if (param.size() < 2)
	{
		client->append_msg_out(ERR_NEEDMOREPARAMS(client->get_nick(), "OPER"));
		return ;
	}
	std::string	target = m.get_params()[0];
	std::string pass = m.get_params()[1];
	Client*	target_cl = get_client(target);
	if (!target_cl)
	{
		client->append_msg_out(ERR_NOSUCHNICK(_host, client->get_nick(), target));
		return ;
	}
	if (pass != _sudo)
	{
		client->append_msg_out(ERR_PASSWDMISMATCH(_host));
		return ;
	}
	target_cl->set_mode("+o");
	target_cl->append_msg_out(RPL_YOUREOPER(_host));
}

void		Server::cmd_kill(Client* client, Message& m)
{
	std::vector<std::string>	param = m.get_params();
	if (param.size() < 1)
	{
		client->append_msg_out(ERR_NEEDMOREPARAMS(client->get_nick(), "KILL"));
		return ;
	}
	std::string	target = m.get_params()[0];
	std::string comment;
	if (m.get_params().size() > 1)
		comment = m.get_params()[1];
	if (!(client->get_mode() & 0b10))
	{
		client->append_msg_out(ERR_NOPRIVILEGES(_host, "KILL"));
		return ;
	}
	Client*	target_cl = get_client(target);
	if (!target_cl)
	{
		client->append_msg_out(ERR_NOSUCHNICK(_host, client->get_nick(), target));
		return;
	}
	std::vector<Channel*>	channels = target_cl->get_channels();
	for (int i = 0; i < (int)channels.size(); i++)
	{
		append_to_channel(target_cl, channels[i], RPL_QUIT(target_cl->get_nick(), target_cl->get_user(), target_cl->get_host(), "QUIT : Killed by " + client->get_nick() + " (" + comment + ")"));
		if (channels[i]->is_oper(target_cl))
			channels[i]->remove_oper(target_cl);
		else
			channels[i]->remove_client(target_cl);
	}
	int	size = _pfds.size();
	int	i = 0;
	for (i = 0; i < size; i++)
	{
		if (_pfds[i].fd == target_cl->get_skFd())
			break;
	}
	_clients.erase(target_cl->get_skFd());
	if (i != size)
		_pfds.erase(_pfds.begin() + i);
	close(target_cl->get_skFd());
}

void		Server::cmd_die(Client *client, Message& m)
{
	std::string	comment;
	if (m.get_params().size() != 0)
		comment = m.get_params()[0];
	if (!(client->get_mode() & 0b10))
	{
		client->append_msg_out(ERR_NOPRIVILEGES(_host, "DIE"));
		return ;
	}
	state = OFF;
}

void		Server::cmd_restart(Client *client, Message& m)
{
	std::string	comment;
	if (m.get_params().size() != 0)
		comment = m.get_params()[0];
	if (!(client->get_mode() & 0b10))
	{
		client->append_msg_out(ERR_NOPRIVILEGES(_host, "RESTART"));
		return ;
	}
	state = RESTART;
}

void		Server::clean_server(void)
{
	//close shit
	for(unsigned long i = 0; i < _pfds.size(); i++)
		close(_pfds[i].fd);
	_pfds.clear();

	//free shit
	std::map<int, Client*>::iterator it;
	for (it = _clients.begin(); it != _clients.end(); ++it)
		delete it->second;
	_clients.clear();
	for(unsigned long i = 0; i < _channels.size(); i++)
		delete _channels[i];
	_channels.clear();
}

void		Server::cmd_quit(Client* client, Message& m)
{
	std::string comment;
	if (m.get_params().size() > 0)
		comment = m.get_params()[0];
	Client*	target_cl = client;
	std::vector<Channel*>	channels = target_cl->get_channels();
	for (int i = 0; i < (int)channels.size(); i++)
	{
		append_to_channel(target_cl, channels[i], RPL_QUIT(target_cl->get_nick(), target_cl->get_user(), target_cl->get_host(), "QUIT :" + comment));
		if (channels[i]->is_oper(target_cl))
			channels[i]->remove_oper(target_cl);
		else
			channels[i]->remove_client(target_cl);
	}
	int	size = _pfds.size();
	int	i = 0;
	for (i = 0; i < size; i++)
	{
		if (_pfds[i].fd == target_cl->get_skFd())
			break;
	}
	_clients.erase(target_cl->get_skFd());
	if (i != size)
		_pfds.erase(_pfds.begin() + i);
	close(target_cl->get_skFd());
	_pfds[i].fd = -1;
}

void		Server::cmd_quot(Client* client, Channel* channel)
{
	client->append_msg_out(RPL_QUOT(_host, channel->get_name(), get_random_quot()));
}

void		Server::send_to_fd(int fd)
{
	if (fd == _svrSk || fd == -1)
		return ;
	Client* client = _clients[fd];
	std::string reply = client->get_msg_out();
	if (reply.empty())
		return;
	if (send(fd, reply.c_str(), reply.size(), 0) == (long)reply.size())
		std::cout << "-->" << reply << "." << std::endl;
	else
		std::cout << "Error: send did not finish" << std::endl;
	client->clear_buff();
}

