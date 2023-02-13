#include "Utils.hpp"

std::vector<std::string>	ft_split(const char buffer[1000], std::string delim)
{
	std::vector<std::string>	vec;

	char *copy = (char *)malloc(strlen(buffer) + 1);
	strcpy(copy, buffer);
    // Pointer to point the word returned by the strtok() function.
    char * p;
    // Here, the delimiter is white space.
    p = strtok(copy, delim.c_str()); 
    while (p != NULL) 
	{
		if (delim == ",")
			vec.push_back(std::string(p));
		else
			vec.push_back(std::string(p).append(delim));
		p = strtok(NULL, delim.c_str());
	}
	return vec;
}
