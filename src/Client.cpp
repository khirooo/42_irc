#include "Client.hpp"

Client::Client(int fd, std::string name)
:
_host(name),
_mode(0),
_usr("*"),
_nick("*"),
_real("*"),
_skFd(fd),
_state(HANDSHAKE)
{
}

Client::~Client()
{
}

std::string	Client::get_host(void) const
{
	return _host;
}

std::string	Client::get_user(void) const
{
	return _usr;
}
std::string	Client::get_nick(void) const
{
	return _nick;
}
std::string	Client::get_real(void) const
{
	return _real;
}

unsigned char	Client::get_mode(void) const
{
	return _mode;
}

std::string	Client::get_mode_str(void) const
{
	std::string	mode = "+";
	if (_mode & 0b01)
		mode += "i";
	if (_mode & 0b10)
		mode += "o";
	return mode;
}


int			Client::get_skFd(void) const
{
	return _skFd;
}
ClientState	Client::get_state(void) const
{
	return _state;
}

std::vector<Channel*>	Client::get_channels(void) const
{
	return _channels;
}

void		Client::set_user(std::string name)
{
	_usr = name;
}

void		Client::set_nick(std::string nick)
{
	_nick = nick;
}

void		Client::set_real(std::string real)
{
	_real = real;
}

void		Client::set_mode(std::string mode)
{
	bool	add = true;

	for (int i = 0; i < mode.size(); i++)
	{
		if (mode[i] == '-')
			add = false;
		else if (mode[i] == '+')
			add = true;
		else if (mode[i] == 'i' && add)
			_mode |= 0b01;
		else if (mode[i] == 'i' && !add)
			_mode &= 0b10; //'10'
		else if (mode[i] == 'o' && add)
			_mode |= 0b10;
		else if (mode[i] == 'o' && !add)
			_mode &= 0b01;
	}
}

void		Client::set_state(ClientState state)
{
	_state = state;
}

void		Client::add_channel(Channel* channel)
{
	_channels.push_back(channel);
}

void		Client::part_channel(Channel* channel)
{
	int	i;
	for(i = 0; i < _channels.size(); i++)
	{
		if (_channels[i]->get_name() == channel->get_name())
			break;
	}
	_channels.erase(_channels.begin() + i);
}

bool	Client::in_channel(std::string channel)
{
	int	i;
	for(i = 0; i < _channels.size(); i++)
	{
		if (_channels[i]->get_name() == channel)
			return true;
	}
	return false;
}
