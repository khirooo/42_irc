#ifndef REPLY_HPP
#define REPLY_HPP

#include "Message.hpp"

//this is soo badd i'm ashamed ...
class Reply:public Message
{

public:
	Reply(std::string prefix, std::string code, std::string comment);
	~Reply();

	std::string	to_str(void) const;
};


#endif