#include "Client.hpp"

Client::Client(int fd)
:
_usr(""),
_nick(""),
_real(""),
_skFd(fd),
_state(HANDSHAKE)
{
}

Client::~Client()
{
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
int			Client::get_skFd(void) const
{
	return _skFd;
}
ClientState	Client::get_state(void) const
{
	return _state;
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
void		Client::set_state(ClientState state)
{
	_state = state;
}
