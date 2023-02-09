#include "Utils.hpp"

std::vector<std::string>	ft_split(char buffer[1000])
{
	std::vector<std::string>	vec;

    // Pointer to point the word returned by the strtok() function.
    char * p;
    // Here, the delimiter is white space.
    p = strtok(buffer, "\r\n"); 
    while (p != NULL) 
	{
		vec.push_back(std::string(p).append("\r\n"));
		p = strtok(NULL, "\r\n");
	}
	return vec;
}
