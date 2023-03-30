#include "Utils.hpp"


std::vector<std::string>	ft_split(const char buffer[1000], std::string delim)
{
	std::vector<std::string>	vec;

	std::string	input = buffer;
	size_t pos = input.find(delim);
	while (pos != std::string::npos)
	{
		vec.push_back(input.substr(0, pos + delim.length()));
		input.erase(0, pos + delim.length());
		pos = input.find(delim);
	}
	if (!input.empty())
		vec.push_back(input);
	return vec;
}

// std::vector<std::string>	ft_split(const char buffer[1000], std::string delim)
// {
// 	std::vector<std::string>	vec;

// 	char *copy = (char *)malloc(strlen(buffer) + 1);
// 	strcpy(copy, buffer);
// 	char * p;
// 	p = strtok(copy, delim.c_str()); 
// 	while (p != NULL) 
// 	{
// 		if (delim == ",")
// 			vec.push_back(std::string(p));
// 		else
// 			vec.push_back(std::string(p).append(delim));
// 		p = strtok(NULL, delim.c_str());
// 	}
// 	free(copy);
// 	return vec;
// }

std::string		check_mode_usr(std::string	mode)
{
	std::string	modes = "i";
	std::string clean_mode;
	bool		found = false;
	for(int i = 0; i < (int)mode.size(); i++)
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

bool	check_chan_name(std::string name)
{
	unsigned long	i;

	for (i = 0; i < name.length(); i++)
	{
		if (static_cast<int>(name[i]) == 7 || name[i] == ',' || name[i] == 	' ')
			return false;
	}
	if (i < 2 || i > 50)
		return false;
	if (name[0] != '#')
		return false;
	return true;
}

bool	check_nick_name(std::string name)
{
	unsigned long	i;

	for (i = 0; i < name.length(); i++)
	{
		if (i == 0 && (name[i] < 'A' || name[i] > '}'))
			return false;
		if (i != 0 && !((name[i] >= 'A' && name [i] <= '}') || (name[i] >= '0' && name [i] <= '9') || name[i] == '-'))
			return false;
	}
	if (i > 9)
		return false;
	return true;
}

std::string		check_mode_chan(std::string	mode, std::string target)
{
	std::string	modes;
	if (target == "")
		modes = "timb";
	else
		modes = "o";
	std::string clean_mode;
	bool		found = false;
	for(int i = 0; i < (int)mode.size(); i++)
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

bool		check_msg(Message m)
{
	std::string	cmd_list[17] = {"OPER", "PASS", "NICK", "USER", "JOIN", "PART", "PRIVMSG", "NOTICE", "QUIT", "NAMES", "MODE", "TOPIC", "KICK", "KILL", "INVITE", "DIE", "RESTART"};

	if (m.get_cmd() == "CAP" || m.get_cmd() == "PING")
		return true;

	if (std::count(cmd_list, cmd_list + 17, m.get_cmd()) == 0)
		return false;

	std::vector<std::string>	params = m.get_params();
	for(std::vector<std::string>::iterator it = params.begin(); it != params.end(); it++)
	{
		if (*it == "")
			return false;
	}
	return true;
}

// void		send_to_client(Client* client, std::string reply)
// {
// 	if (send(client->get_skFd(), reply.c_str(), reply.size(), 0) == (long)reply.size())
// 		std::cout << "-->" << reply << "." << std::endl;
// 	else
// 		std::cout << "Error: send did not finish" << std::endl;
// }

void		append_to_channel(Client* client, Channel* channel, std::string reply)
{
	std::vector<Client*>	clients = channel->get_clients();
	for(int	i= 0; i < (int)clients.size(); i++)
	{
		if (clients[i]->get_nick() == client->get_nick())
			continue;
		clients[i]->append_msg_out(reply);
	}
}

// void		send_to_channel(Client* client, Channel* channel, std::string reply)
// {
// 	std::vector<Client*>	clients = channel->get_clients();
// 	for(int	i= 0; i < (int)clients.size(); i++)
// 	{
// 		std::cout << "in channel:" << clients[i]->get_nick() << std::endl;
// 		if (clients[i]->get_nick() == client->get_nick())
// 			continue;
// 		send_to_client(clients[i], reply);
// 	}
// }

bool			is_in(int fd, std::vector<struct pollfd>	pfds)
{
	for (unsigned long i = 0; i < pfds.size(); i++)
	{
		if (pfds[i].fd == fd)
			return true;
	}
	return false;
}
std::string		get_random_quot(void)
{
	std::vector<std::string>	quots_db;
	quots_db.push_back("“We cannot solve problems with the kind of thinking we employed when we came up with them.” — Albert Einstein");
	quots_db.push_back("“Learn as if you will live forever, live like you will die tomorrow.” — Mahatma Gandhi");
	quots_db.push_back("“Stay away from those people who try to disparage your ambitions. Small minds will always do that, but great minds will give you a feeling that you can become great too.” — Mark Twain");
	quots_db.push_back("“When you give joy to other people, you get more joy in return. You should give a good thought to happiness that you can give out.”— Eleanor Roosevelt");
	quots_db.push_back("“When you change your thoughts, remember to also change your world.”—Norman Vincent Peale");
	quots_db.push_back("“It is only when we take chances, when our lives improve. The initial and the most difficult risk that we need to take is to become honest. —Walter Anderson");
	quots_db.push_back("“Nature has given us all the pieces required to achieve exceptional wellness and health, but has left it to us to put these pieces together.”—Diane McLaren");
	quots_db.push_back("“Success is not final; failure is not fatal: It is the courage to continue that counts.” — Winston S. Churchill");
	quots_db.push_back("“It is better to fail in originality than to succeed in imitation.”— Herman Melville");
	quots_db.push_back("“The road to success and the road to failure are almost exactly the same.” — Colin R");
	quots_db.push_back("“Success usually comes to those who are too busy looking for it.” — Henry David Thoreau");
	quots_db.push_back("“Develop success from failures. Discouragement and failure are two of the surest stepping stones to success.”—Dale Carnegie");
	quots_db.push_back("“Ain't no sunshine when she's gone.” — Robel Kedida");
	quots_db.push_back("“Fuck power points, there is no power in those points, and no points in those presentations.” — Akshay");
	quots_db.push_back("“if someone tries to do smth funny let me talk” — Moad");
	std::srand(std::time(0));
	std::size_t index = std::rand() % quots_db.size();
	return quots_db[index];
}