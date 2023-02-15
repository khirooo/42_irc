#include "Server.hpp"

int	main(void)
{
	Server	my_server("pass", "6667");
	bool	keep_running = true;
	while (keep_running)
	{
		try
		{
			my_server.initilize();
			my_server.start();
		}
		catch(Server::ShutDownException& e)
		{
			std::cerr << e.what() << std::endl;
			keep_running = false;
		}
		catch(Server::RestartException& e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
	
	return 0;
}