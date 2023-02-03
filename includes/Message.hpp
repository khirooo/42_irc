#ifndef MESSAGE_HPP
#define	MESSAGE_HPP

#include <string>

class Message
{
private:
	std::string	msg; //the whole thing
	std::string	prefix;//starts with ':' followed by sender eg. :doctor!doctor@baz.example.org ...
	std::string	command; //1 word or 3digits (reply)
	std::string param[14]; //comand parameters
public:
	Message(char* buffer);
	~Message();
};




#endif