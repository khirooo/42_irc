#include "Channel.hpp"

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

std::vector<Client*>	Channel::get_clinets(void) const
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
