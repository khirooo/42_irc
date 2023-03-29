#include "Message.hpp"
#include <sstream>

std::string	crlf = "\r\n";

Message::Message()
:
_msg(""),
_prefix(""),
_cmd(""),
_complete(false)
{
}

Message::~Message()
{
}

void	Message::append(const char* buffer)
{
	_msg+=std::string(buffer);
	if (_msg.size() < 2)
		return ;
	_complete = std::equal(crlf.rbegin(), crlf.rend(), _msg.rbegin());
	if (_complete)
		parse_msg();
}

void	Message::parse_msg(void)
{
	std::istringstream in_stream(_msg.substr(0, _msg.size() - 2));
	//get the prefix 
	if (_msg[0] == ':')
		std::getline(in_stream, _prefix, ' ');

	in_stream >> _cmd; //get the command

	std::string param;
	while (std::getline(in_stream, param, ' '))
	{
		if (param[0] == ':') {
			std::string  tmp;
			std::getline(in_stream, tmp, '\r');
			if (tmp.empty())
				_params.push_back(param.substr(1));
			else
				_params.push_back(param.substr(1) + ' ' + tmp);
			break;
		}
		if (!param.empty())
			_params.push_back(param);
	}
}

bool	Message::is_complete(void) const
{
	return _complete;
}

std::string	Message::get_msg(void) const
{
	return _msg;
}

std::string	Message::get_prefix(void) const
{
	return _prefix;
}

std::string	Message::get_cmd(void) const
{
	return _cmd;
}

std::vector<std::string>	Message::get_params(void) const
{
	return _params;
}

void		Message::clear(void)
{
	_msg = "";
	_prefix = "";
	_cmd = "";
	_params.clear();
	_complete = false;
}