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
    Socket(const int &host, const int &port);
    ~Socket(void);

    Socket &operator=(const Socket &other);
    void close(void);
    int getFd(void) const;
    int accept(void);

  private:
    int _fd;
};
