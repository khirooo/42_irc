#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <fcntl.h>
#include <map>
#include <vector>
#include <poll.h>
#include <string>
int main(void)
{

	int						_svrSk;		// main server socket (for clients to connect)
	std::vector<struct pollfd>	_pfds;
	struct addrinfo hints, *servAddr, *p;
	int	yes = 1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; //using both ip4 and ip6
	hints.ai_socktype = SOCK_STREAM; //socket type
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, "6668", &hints, &servAddr))
	{
		std::cerr << "Error: getaddrinfo " << std::endl;
		exit(1); //!!!! handel ur shit here
	}

	for (p = servAddr; p != NULL; p = p->ai_next)
	{
		std::cout << "once\n";
		_svrSk = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (_svrSk < 0)
		{
			std::cerr << "Error: socket " <<  std::strerror(errno) << std::endl;
			continue; //try next socket
		}

		if (setsockopt(_svrSk, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
		{
        	std::cerr << "Error: setsockopt " << strerror(errno) << std::endl;
        	continue ;
		}
		int ret = bind(_svrSk, p->ai_addr, p->ai_addrlen);
		if (ret == -1)
		{
			std::cerr << "Error: bind " << strerror(errno) << std::endl;
			close(_svrSk);
			continue;
		}
		else
			std::cout << "binded the server socket " << _svrSk << " with the port 6668(ret = " << ret << ")" << std::endl;
		
		break; //success
	}
	freeaddrinfo(servAddr);
	if (p == NULL)
	{
		std::cerr << "Error: not able to bind a socket ... tneket" << std::endl;
		exit (1); //!!!! handel ur shit here
	}
	if (listen(_svrSk, 10) == -1)
	{
		std::cerr << "Error: listen " << std::strerror(errno) << std::endl;
		exit(1); //!!!! handel ur shit
	}
	struct pollfd pfds_arr[1];
	pfds_arr[0].fd = _svrSk;
	pfds_arr[0].events = POLLIN;
	int	exit_code;
	int	size = 1;
	struct sockaddr_storage address;
	socklen_t address_len = sizeof(address);

	if (getsockname(_svrSk, (struct sockaddr *) &address, &address_len) == -1) {
		std::cerr << "getsockname" << std::endl;
		exit(EXIT_FAILURE);
	}

	char host[NI_MAXHOST];
	char service[NI_MAXSERV];

	if (getnameinfo((struct sockaddr *) &address, address_len, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV) == 0) {
		std::cout << "Listening on host= " << host << ", " << service << std::endl;
	} else {
		std::cerr <<"getnameinfo" << std::endl;
		exit(EXIT_FAILURE);
	}
	while (true)
	{
		// struct pollfd* pfds_arr = new struct pollfd[size];
		// std::copy(_pfds.begin(), _pfds.end(), pfds_arr);
		if (pfds_arr[0].revents & POLLHUP)
			std::cout << "Mel bedya m9awda \n";
		std::cout << "before poll: sockedID " << _svrSk << " events " << pfds_arr[0].events  << " revent "<< pfds_arr[0].revents << std::endl;

		exit_code = poll(pfds_arr, size, -1);
		if (pfds_arr[0].revents & POLLIN)
			std::cout << "NIKMOK\n";
		getchar();
		if (exit_code == -1) //block indefinitely
		{
			std::cerr << "Error: poll " << strerror(errno) << std::endl;
			exit(1); //!!!! handel ur shit here
		}
		else
		{
			std::cerr << "Error: poll timeout " << exit_code << " erron " << errno<< std::endl;
		}
		std::cout << "after poll" << std::endl;
	}
	return 0;
}