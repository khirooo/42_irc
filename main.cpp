#include "Server.hpp"

int	main(void)
{
	Server	my_server("pass", "6667");
	while (my_server.state == RESTART)
	{
		my_server.initilize();
		my_server.start();
		//free & close stuffß
	}
	return 0;
}