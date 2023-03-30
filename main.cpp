#include "Server.hpp"

int	main(int argc, char **argv)
{
	std::string	password;
	std::string	port;

	if (argc != 2 && argc != 3)
	{
		write(STDOUT_FILENO, "Usage: ./ircserv [<port>] <password>\n", 37);//when dont touch c++ for a while :)
		return 1;
	}
	if (argc == 2)
	{
		port = "6667";
		password = argv[1];
	}
	else
	{
		port = argv[1];
		password = argv[2];
	}
	Server	my_server(password, port);
	while (my_server.state == RESTART)
	{
		my_server.initilize();
		my_server.start();
		my_server.clean_server();
		sleep(2);
	}
	return 0;
}
