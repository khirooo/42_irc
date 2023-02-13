#include "Channel.hpp"
#include <iostream>
Channel::Channel(const std::string name)
:
_name(name),
_mode(0b110)
{
}

Channel::~Channel()
{
}

std::string	Channel::get_name(void) const
{
	return _name;
}

unsigned char Channel::get_mode(void) const
{
	return _mode;
}

std::vector<Client*>	Channel::get_clients(void) const
{
	return _clients;
}

std::string				Channel::get_topic(void) const
{
	return _topic;
}

unsigned int	Channel::get_n_clients(void) const
{
	return _clients.size();
}

std::string		Channel::get_clients_nick(Client* client) const
{
	std::string	clients_ls;
	for(int i = 0; i < _clients.size(); i++)
		clients_ls += " " + _clients[i]->get_nick();
	clients_ls += "\r\n";
	return clients_ls;
}

void	Channel::set_mode(std::string mode)
{
	bool	add = true;
	for (int i = 0; i < mode.size(); i++)
	{
		if (mode[i] == '-')
			add = false;
		else if (mode[i] == '+')
			add = true;
		else if (mode[i] == 'm' && add)
			_mode |= 0b001;
		else if (mode[i] == 'm' && !add)
			_mode &= 0b110;
		else if (mode[i] == 'l' && add)
			_mode |= 0b010;
		else if (mode[i] == 'l' && !add)
			_mode &= 0b101;
		else if (mode[i] == 't' && add)
			_mode |= 0b100;
		else if (mode[i] == 't' && !add)
			_mode &= 0b011;
	}
}

std::string	Channel::get_mode_str(void) const
{
	std::string	mode = "+";
	if (_mode & 0b001)
		mode += "m";
	if (_mode & 0b010)
		mode += "l";
	if (_mode & 0b100)
		mode += "t";
	return mode;
}

void	Channel::set_topic(std::string topic)
{
	_topic = topic;
}

void	Channel::add_client(Client* client)
{
	_clients.push_back(client);
}

void	Channel::add_oper(Client* client)
{
	_opers.push_back(client);
}

void	Channel::remove_client(Client* client)
{
	int i;
	for (i = 0; i < _clients.size(); i++)
	{
		if (_clients[i]->get_skFd() == client->get_skFd())
			break;
	}
	_clients.erase(_clients.begin() + i);
}

bool	Channel::is_oper(Client* client) const
{
	return true;
	// for(int i = 0; i < _opers.size(); i++)
	// {
	// 	if (_opers[i]->get_nick() == client->get_nick())
	// 		return true;
	// }
	// return false;
}

/*--------debug stuff---------*/

void	Channel::print_clients()
{
	for (int i = 0; i < _clients.size(); i++)
		std::cout << _clients[i]->get_nick() << " ";
	std::cout << std::endl;
}