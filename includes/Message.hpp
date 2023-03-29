#ifndef MESSAGE_HPP
#define	MESSAGE_HPP

#include <string>
#include <vector>
#include <iostream>

class Message
{
protected:
	std::string					_msg; //the whole thing
	std::string					_prefix;//starts with ':' followed by sender eg. :doctor!doctor@baz.example.org ...
	std::string					_cmd; //1 word
	std::vector<std::string>	_params; //comand parameters
	bool						_complete; //if the msg containe portion of the message and not all of it (it does not end with \r\n)
public:
	Message();
	~Message();

	void						append(const char* buffer);
	void						parse_msg(void);

	std::string					get_msg(void) const;
	std::string					get_prefix(void) const;
	std::string					get_cmd(void) const;
	std::vector<std::string>	get_params(void) const;
	bool						is_complete(void) const;
	void						clear(void);
};

#endif