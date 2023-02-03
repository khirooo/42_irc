#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <string>

enum ClientState {
	HANDSHAKE,
	LOGIN,
	REGISTERED,
	DISCONNECTED
};


class Client
{
private:
	std::string	_usr;
	std::string	_nick;
	std::string	_real;
	int			_skFd;
	ClientState	_state;
public:
	Client();
	~Client();
};




#endif