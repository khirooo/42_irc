#include "Utils.hpp"

std::vector<std::string>	ft_split(const char buffer[1000], std::string delim)
{
	std::vector<std::string>	vec;

	char *copy = (char *)malloc(strlen(buffer) + 1);
	strcpy(copy, buffer);
    char * p;
    p = strtok(copy, delim.c_str()); 
    while (p != NULL) 
	{
		if (delim == ",")
			vec.push_back(std::string(p));
		else
			vec.push_back(std::string(p).append(delim));
		free(p);
		p = strtok(NULL, delim.c_str());
	}
	return vec;
}
