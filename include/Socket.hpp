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
    struct sockaddr_in *getAddress(void);
    int *getAddressLen(void);
    int accept();

  private:
    int _fd;
    int _addressLen;
    struct sockaddr_in _address;
};
