#pragma once

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

class Socket
{
  public:
    Socket(const std::string &host, const std::string &port);
    ~Socket(void);
    void close(void);
    int getFd(void) const;

  private:
    int _fd;
    struct sockaddr_in _address;
};
