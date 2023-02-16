#include "Utils.hpp"


std::vector<std::string>	ft_split(const char buffer[1000], std::string delim)
{
	std::vector<std::string>	vec;

	char *copy = (char *)malloc(strlen(buffer) + 1);
	strcpy(copy, buffer);
	char * p;
	p = strtok(copy, delim.c_str()); 
	while (p != NULL) 
	{
		if (delim == ",")
			vec.push_back(std::string(p));
		else
			vec.push_back(std::string(p).append(delim));
		//free(p);
		p = strtok(NULL, delim.c_str());
	}
	return vec;
}

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

std::string		check_mode_chan(std::string	mode, std::string target)
{
	std::string	modes;
	if (target == "")
		modes = "tim";
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

void		send_to_client(Client* client, std::string reply)
{
	if (send(client->get_skFd(), reply.c_str(), reply.size(), 0) == (long)reply.size())
		std::cout << "-->" << reply << "." << std::endl;
	else
		std::cout << "Error: send did not finish" << std::endl;
}

void		send_to_channel(Client* client, Channel* channel, std::string reply)
{
	std::vector<Client*>	clients = channel->get_clients();
	for(int	i= 0; i < (int)clients.size(); i++)
	{
		if (clients[i]->get_nick() == client->get_nick())
			continue;
		send_to_client(clients[i], reply);
	}
}
