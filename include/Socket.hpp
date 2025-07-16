#pragma once

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>

class Socket
{
   public:
	Socket(const std::string &host, const std::string &port);
	~Socket(void);
	void close(void);
	int getFd(void) const;
	int accept(void);

   private:
	int _fd;
};
