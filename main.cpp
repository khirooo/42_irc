#include "Server.hpp"

int	main(void)
{
	Server	my_server("passwd", "6667");
	my_server.initilize();
	my_server.start();
	return 0;
}