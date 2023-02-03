#include "Message.hpp"

Message::Message(char* buffer)
{
	msg = std::string(buffer);
	//handel all the other shit here ... 
}

Message::~Message()
{
}