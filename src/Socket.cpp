#include "Socket.hpp"
#include <iostream>

Socket::Socket(const int &host, const int &port)
{
    struct sockaddr_in address;
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd < 0)
        throw std::runtime_error("Socket: failed to create socket");

    int opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        throw std::runtime_error("Socket: failed to set socket opt");

    std::cout << host << port << std::endl;
    address.sin_family = AF_INET;
    if (host == 0)
        address.sin_addr.s_addr = INADDR_ANY;
    else
        address.sin_addr.s_addr = htonl(host);
    address.sin_port = htons(port);
    std::memset(address.sin_zero, '\0', sizeof(address.sin_zero));

    if (bind(_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
        throw std::runtime_error("Socket: failed to bind socket");

    int flags = fcntl(_fd, F_GETFL, 0);
    if (flags < 0)
        throw std::runtime_error("Socket: failed to get socket flags");

    if (fcntl(_fd, F_SETFL, flags | O_NONBLOCK) < 0)
        throw std::runtime_error("Socket: failed to make socket non-blocking");

    if (listen(_fd, SOMAXCONN) < 0)
        throw std::runtime_error("Socket: a socket failed to listen");
}

Socket::~Socket(void)
{
    close();
}

void Socket::close(void)
{
    ::close(_fd);
}

Socket &Socket::operator=(const Socket &other)
{
    _fd = other._fd;
    return (*this);
}

int Socket::getFd(void) const
{
    return (_fd);
}

int Socket::accept(void)
{
    struct sockaddr_in clientAdress;
    int clientAdressLen = sizeof(clientAdress);
    int clientFd = ::accept(_fd, (struct sockaddr *)&clientAdress, (socklen_t *)&clientAdressLen);

    if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
    {
        ::close(clientFd);
        return (-1);
    }

    return (clientFd);
}
