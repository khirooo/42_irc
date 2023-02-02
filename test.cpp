#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>

int main() {
  int status;
  int listen_socket;
  struct addrinfo hints;
  struct addrinfo *servinfo;  // will point to the results

  memset(&hints, 0, sizeof hints); // make sure the struct is empty
  hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
  hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

  status = getaddrinfo(NULL, "6668", &hints, &servinfo);
  if (status != 0) {
    std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
    return 1;
  }

  listen_socket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
  if (listen_socket == -1) {
    std::cerr << "socket error" << std::endl;
    return 1;
  }

  int yes = 1;
  status = setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  if (status == -1) {
    std::cerr << "setsockopt error" << std::endl;
    return 1;
  }

  status = bind(listen_socket, servinfo->ai_addr, servinfo->ai_addrlen);
  if (status == -1) {
    std::cerr << "bind error" << std::endl;
    return 1;
  }

  status = listen(listen_socket, 10);
  if (status == -1) {
    std::cerr << "listen error" << std::endl;
    return 1;
  }

  // using poll to listen for incoming connections
  struct pollfd fds[1];
  int timeout_msecs = 5000;

  fds[0].fd = listen_socket;
  fds[0].events = POLLIN;

  while (true) {
    int poll_result = poll(fds, 1, timeout_msecs);
    if (poll_result == -1) {
      std::cerr << "poll error" << std::endl;
      return 1;
    }
    if (poll_result == 0) {
      std::cout << "poll timeout" << std::endl;
      continue;
    }
    if (fds[0].revents & POLLIN) {
      std::cout << "someone is here" << std::endl;
    }
  }

  return 0;
}
