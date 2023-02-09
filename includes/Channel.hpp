#ifndef CHANNEL_HPP
#define	CHANNEL_HPP

#include "Client.hpp"


class Client;

class	Channel
{
private:
	
	std::string				_name;
	std::vector<Client*>	_clients;
	std::vector<Client*>	_opers;
public:
	Channel(const std::string name);
	~Channel();

	std::string				get_name(void) const;
	std::vector<Client*>	get_clinets(void) const; //weird...(EDIT: not really)
	void					add_client(Client* client);
	void					add_oper(Client* clinet);
	//get n clinet
	//get nicks
	//get topic ...ect
};




#endif