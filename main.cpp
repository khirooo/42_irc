#include "Server.hpp"

int	main(void)
{
	Server	my_server("passwd", "6669");
	my_server.initilize();
	my_server.start();
	return 0;
}