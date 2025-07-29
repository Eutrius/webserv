#pragma once

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>
#include "parser.hpp"

class Socket
{
   public:
	Socket(void);
	~Socket(void);
	Socket &operator=(const Socket &other);

	int getFd(void) const;
	std::vector<Server> getServers(void) const;

	void init(t_host host, std::vector<Server> servers);
	int accept(void);

	static std::vector<Socket> initSockets(t_serversMap serverMaps);

   private:
	int _fd;
	std::vector<Server> _servers;
};
