#include "Channel.hpp"
#include <iostream>
Channel::Channel(const std::string name)
:
_name(name)
{
}

Channel::~Channel()
{
}

std::string	Channel::get_name(void) const
{
	return _name;
}

std::vector<Client*>	Channel::get_clients(void) const
{
	return _clients;
}

void	Channel::add_client(Client* client)
{
	_clients.push_back(client);
}

void	Channel::add_oper(Client* client)
{
	_opers.push_back(client);
}

unsigned int	Channel::get_n_clients(void) const
{
	return _clients.size();
}


std::string		Channel::get_clients_nick(void) const
{
	std::string	clients_ls;
	for(int i = 0; i < _clients.size(); i++)
		clients_ls += " " + _clients[i]->get_nick();
	clients_ls += "\r\n";
	return clients_ls;
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


/*--------debug stuff---------*/

void	Channel::print_clients()
{
	for (int i = 0; i < _clients.size(); i++)
		std::cout << _clients[i]->get_nick() << " ";
	std::cout << std::endl;
}