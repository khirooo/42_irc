#include "Server.hpp"

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

Server::Server(string pass, string port)
:
_port(port),
_password(pass)
{
}

Server::~Server()
{
}

void	Server::initilize(void)
{
	int	exit_code;
	int	yes = 1;
	struct addrinfo hints, *servAddr, *p;

	//-- filling some hints for getaddrinfo so that it generates a addinfo struct for me
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; //using both ip4 and ip6
	hints.ai_socktype = SOCK_STREAM; //socket type
	hints.ai_flags = AI_PASSIVE; //fill my ip for me (if i pass NULL as node for getaddrinfo)

	exit_code = getaddrinfo(NULL, _port.c_str(), &hints, &servAddr);
	if (exit_code == -1)
	{
		std::cerr << "Error: getaddrinfo " << gai_strerror(exit_code) << std::endl;
		exit(1); //!!!! handel ur shit here
	}

	//-- looping throw the lists of addinfos until a creation(a bind) succeed 
	for (p = servAddr; p != NULL; p = p->ai_next)
	{
		// create a socket
		_svrSk = socket(p->ai_family, p->ai_socktype, p->ai_protocol); //ip4,sock_stream, get the default protocol for sock_stream (tcp)
		if (_svrSk == -1)
		{
			std::cerr << "Error: socket " <<  std::strerror(errno) << std::endl;
			continue; //try next socket
		}

		// reuse the address if it's used 
		if (setsockopt(_svrSk, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
		{
        	std::cerr << "Error: setsockopt " << strerror(errno) << std::endl;
        	continue ;
		}

		//bind the socket
		exit_code = bind(_svrSk, p->ai_addr, p->ai_addrlen);
		if (exit_code)
		{
			std::cerr << "Error: bind " << strerror(errno) << std::endl;
			close(_svrSk);
			continue;
		}
		else
			std::cout << "binded the server socket " << _svrSk << " with the port " << _port << std::endl;
		break; //success
	}
	freeaddrinfo(servAddr);
	if (p == NULL)
	{
		std::cerr << "Error: not able to bind a socket ... tneket" << std::endl;
		exit (1); //!!!! handel ur shit here
	}
	//listen to the socket dumb fuck
	if (listen(_svrSk, 10) == -1)
	{
		std::cerr << "Error: listen " << std::strerror(errno) << std::endl;
		exit(1); //!!!! handel ur shit
	}
	//--adding server socket to pollfd array
	struct pollfd	pfd;
	pfd.fd = _svrSk;
	pfd.events = POLLIN;
	_pfds.push_back(pfd); //_pfds.insert(_pfds.begin(), pfd);
	//NOTE : u might need to add a pollfd for STDIN for handelling server commands
}

void	Server::start(void)
{
	int	exit_code;

	while (true)
	{
		int	size = _pfds.size();
		struct pollfd* pfds_arr = new struct pollfd[size];
		std::copy(_pfds.begin(), _pfds.end(), pfds_arr);
		std::cout << "_____POLL____"<< size << "____" << std::endl ;
		exit_code = poll(pfds_arr, size, -1);//block indefinitely
		if (exit_code == -1)
		{
			std::cerr << "Error: poll " << strerror(errno) << std::endl;
			exit(1); //!!!! handel ur shit here
		}

		//loop throught the pfds and check the events NOTE: break at poll return (faster!)
		for(int i = 0; i < size; i++)
		{
			if (pfds_arr[i].revents & POLLIN) //we got an event here!
			{
				if (pfds_arr[i].fd == _svrSk) //it's a new conncetion SO EXCITTEDDDD!!
					accept_connection();
				else //it's a new message from a client
					handel_message(pfds_arr, i);
			}
		}
		delete[] pfds_arr;
	}
}

void	Server::accept_connection()
{
	int						new_sock;
	struct sockaddr_storage	client_addr;
	socklen_t				new_sock_len = sizeof(client_addr);

	new_sock = accept(_svrSk, (sockaddr*)&client_addr, &new_sock_len); //welcome b*t*h
	if (new_sock == -1)
		std::cerr << "Error: accept " << std::strerror(errno) << std::endl;
	else
		std::cout << "new connection so Excitedd!" << std::endl;
	struct pollfd pfd;
	pfd.fd = new_sock;
	pfd.events = POLLIN;
	_pfds.push_back(pfd);
	//NOTE : u might need to update the array here ! if u want ur server to be fast
}

void	Server::handel_message(struct pollfd* pfds_arr, int i)
{
	int	exit_code;

	memset(_buffer, 0, BUFFER_SIZE);
	exit_code = recv(pfds_arr[i].fd, _buffer, BUFFER_SIZE, 0); //flags = MSG_DONTWAIT might be faster ...

	if (exit_code <= 0) //connection closed or fail
	{
		if (exit_code == 0)
			std::cout << "Connection closed\n";
		else if (exit_code == -1)
			std::cerr << "Error: recv " << std::strerror(errno) << std::endl;
		close(pfds_arr[i].fd); //chuss
		_pfds.erase(_pfds.begin() + i);
		//NOTE : remove it from pfd_arr ...
	}
	else
	{
		std::cout << "Recived message: " << _buffer << std::endl;	
	}
}


