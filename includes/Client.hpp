#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <string>

enum ClientState {
	HANDSHAKE, 		// connected to the socket
	LOGIN,    		// correct PASS given
	REGISTERED,		// executed USER & NICK
	DISCONNECTED	// wasted (donno why i need this acctually)
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
	Client(int fd);
	~Client();
	std::string	get_user(void) const;
	std::string	get_nick(void) const;
	std::string	get_real(void) const;
	int			get_skFd(void) const;
	ClientState	get_state(void) const;

	void		set_user(std::string name);
	void		set_nick(std::string nick);
	void		set_real(std::string real);
	void		set_state(ClientState state);
};




#endif