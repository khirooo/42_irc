#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <string>
#include <vector>

#include "Channel.hpp"

enum ClientState {
	HANDSHAKE, 		// connected to the socket
	LOGIN,	 		// correct PASS given
	REGISTERED,		// executed USER & NICK
	DISCONNECTED	// wasted (donno why i need this acctually)
};

class	Channel;

class Client
{

private:
	std::string				_host;
	std::string				_usr;
	std::string				_nick;
	std::string				_real;
	std::string				_mode;
	int						_skFd;
	ClientState				_state;
	std::vector<Channel*>	_channels;

public:
	Client(int fd, std::string name);
	~Client();
	std::string				get_host(void) const;
	std::string				get_user(void) const;
	std::string				get_nick(void) const;
	std::string				get_real(void) const;
	std::string				get_mode(void) const;
	int						get_skFd(void) const;
	ClientState				get_state(void) const;
	std::vector<Channel*>	get_channels(void) const;

	void		set_user(std::string name);
	void		set_nick(std::string nick);
	void		set_real(std::string real);
	void		set_mode(std::string mode);
	void		set_state(ClientState state);
	void		add_channel(Channel* channel);
	void		part_channel(Channel* channel);
	bool		in_channel(std::string channel);
};




#endif