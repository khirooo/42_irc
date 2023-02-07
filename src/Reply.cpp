#include "Reply.hpp"

Reply::Reply(std::string prefix, std::string code, std::string comment)
: Message()
{
	_prefix = prefix;
	_cmd = code;
	if (!comment.empty())
		_params.push_back(comment);
}

Reply::~Reply()
{
}

std::string	Reply::to_str(void) const
{
	std::string comment = "";
	std::string	prf = _prefix;
	if (!_prefix.empty())
		prf = ":" + _prefix;
	if (!_params.empty())
		comment = ":" + *_params.begin();
	return prf + " " + _cmd + " " + comment;
}
