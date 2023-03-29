#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <string>
#include <vector>

#include "Channel.hpp"
#include "Message.hpp"

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
	unsigned char			_mode;
	int						_skFd;
	ClientState				_state;
	Message					_msg_in;
	std::string				_msg_out;
	std::vector<Channel*>	_channels;
	std::vector<Channel*>	_invites;

public:
	Client(int fd, std::string name);
	~Client();
	std::string				get_host(void) const;
	std::string				get_user(void) const;
	std::string				get_nick(void) const;
	std::string				get_real(void) const;
	unsigned char			get_mode(void) const;
	std::string				get_mode_str(void) const;
	int						get_skFd(void) const;
	ClientState				get_state(void) const;
	std::vector<Channel*>	get_channels(void) const;
	Message&				get_msg_in(void);
	std::string				get_msg_out(void);

	void		append_msg_out(std::string msg);
	void		set_user(std::string name);
	void		set_nick(std::string nick);
	void		set_real(std::string real);
	void		set_mode(std::string mode);
	void		set_state(ClientState state);
	void		add_channel(Channel* channel);
	void		add_invite(Channel* channel);
	void		part_channel(Channel* channel);
	void		remove_invite(Channel* channel);
	bool		in_channel(std::string channel);
	bool		is_invited(std::string channel) const;
	void		clear_buff(void);
};


#endif