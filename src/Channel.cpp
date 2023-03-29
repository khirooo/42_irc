#include "Channel.hpp"
#include <iostream>
Channel::Channel(const std::string name)
:
_name(name),
_mode(0b0110)
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
	std::vector<Client*>	vec = _clients;
	vec.insert(vec.begin(), _opers.begin(), _opers.end());
	return vec;
}

std::string				Channel::get_topic(void) const
{
	return _topic;
}

unsigned int	Channel::get_n_clients(void) const
{
	return _clients.size();
}

std::string		Channel::get_clients_nick(char mode) const
{
	std::string	clients_ls;
	for(unsigned long i = 0; i < _opers.size(); i++)
		if ((mode & 0b01) || !(_opers[i]->get_mode() & 0b01))
			clients_ls += "@" + _opers[i]->get_nick() + " ";
	for(unsigned long i = 0; i < _clients.size(); i++)
		if ((mode & 0b01) || !(_clients[i]->get_mode() & 0b01))
			clients_ls += _clients[i]->get_nick() + " ";
	clients_ls += "\r\n";
	return clients_ls;
}

void	Channel::set_mode(std::string mode, Client* target)
{
	bool	add = true;

	if (target != NULL)
	{
		for (unsigned long i = 0; i < mode.size(); i++)
		{
			if (mode[i] == '-')
				add = false;
			else if (mode[i] == '+')
				add = true;
			if (mode[i] == 'o' && add && !is_oper(target))
			{
				remove_client(target);
				add_oper(target);
			}
			if (mode[i] == 'o' && !add && is_oper(target))
			{
				remove_oper(target);
				add_client(target);
			}
		}
	}
	else
	{
		for (unsigned long i = 0; i < mode.size(); i++)
		{
			if (mode[i] == '-')
				add = false;
			else if (mode[i] == '+')
				add = true;
			else if (mode[i] == 'm' && add)
				_mode |= 0b001;
			else if (mode[i] == 'm' && !add)
				_mode &= 0b110;
			else if (mode[i] == 'i' && add)
				_mode |= 0b010;
			else if (mode[i] == 'i' && !add)
				_mode &= 0b101;
			else if (mode[i] == 't' && add)
				_mode |= 0b100;
			else if (mode[i] == 't' && !add)
				_mode &= 0b011;
			else if (mode[i] == 'b' && add)
				_mode |= 0b1000;
			else if (mode[i] == 'b' && !add)
				_mode &= 0b0111;
		}
	}
}

std::string	Channel::get_mode_str(void) const
{
	std::string	mode = "+";
	if (_mode & 0b0001)
		mode += "m";
	if (_mode & 0b0010)
		mode += "i";
	if (_mode & 0b0100)
		mode += "t";
	if (_mode & 0b1000)
		mode += "b";
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
	unsigned long i;
	for (i = 0; i < _clients.size(); i++)
	{
		if (_clients[i]->get_skFd() == client->get_skFd())
			break;
	}
	if (i != _clients.size())
		_clients.erase(_clients.begin() + i);
}

void	Channel::remove_oper(Client* client)
{
	unsigned long i;
	for (i = 0; i < _opers.size(); i++)
	{
		if (_opers[i]->get_skFd() == client->get_skFd())
			break;
	}
	if (i != _opers.size())
		_opers.erase(_opers.begin() + i);
}

bool	Channel::is_oper(Client* client) const
{
	for(unsigned long i = 0; i < _opers.size(); i++)
	{
		if (_opers[i]->get_nick() == client->get_nick())
			return true;
	}
	return false;
}
