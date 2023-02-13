#ifndef CHANNEL_HPP
#define	CHANNEL_HPP

#include "Client.hpp"


class Client;

class	Channel
{
private:
	
	std::string				_name;
	unsigned char			_mode;
	std::vector<Client*>	_clients;
	std::vector<Client*>	_opers;
	std::string				_topic;
public:
	Channel(const std::string name);
	~Channel();

	std::string				get_name(void) const;
	unsigned char			get_mode(void) const;
	std::string				get_mode_str(void) const;
	std::vector<Client*>	get_clients(void) const;
	std::string				get_topic(void) const;
	unsigned int			get_n_clients(void) const;
	std::string				get_clients_nick(Client* client) const;

	void					set_mode(std::string mode);
	void					set_topic(std::string topic);
	void					add_client(Client* client);
	void					add_oper(Client* clinet);
	void					remove_client(Client* client);
	bool					is_oper(Client* client) const;

	/*------debug stuff-----*/
	void	print_clients();
};




#endif