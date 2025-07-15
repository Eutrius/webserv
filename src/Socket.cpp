#include "Socket.hpp"

static int atoi_ip(const std::string &host);

Socket::Socket(const std::string &host, const std::string &port)
{
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd < 0)
        throw std::runtime_error("Socket: failed to create socket");

    int opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        throw std::runtime_error("Socket: failed to set socket opt");

    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = htonl(atoi_ip(host));
    _address.sin_port = htons(std::atoi(port.c_str()));
    std::memset(_address.sin_zero, '\0', sizeof(_address.sin_zero));

    if (bind(_fd, (struct sockaddr *)&_address, sizeof(_address)) < 0)
        throw std::runtime_error("Socket: failed to bind socket");

    int flags = fcntl(_fd, F_GETFL, 0);
    if (flags < 0)
        throw std::runtime_error("Socket: failed to get socket flags");

    if (fcntl(_fd, F_SETFL, flags | O_NONBLOCK) < 0)
        throw std::runtime_error("Socket: failed to make socket non-blocking");

    if (listen(_fd, SOMAXCONN) < 0)
        throw std::runtime_error("Socket: failed to listen to socket at " + host + ":" + port);
}

Socket::~Socket(void)
{
    close();
}

void Socket::close(void)
{
    ::close(_fd);
}

int Socket::getFd(void) const
{
    return (_fd);
}

static int atoi_ip(const std::string &host)
{
    std::istringstream ss(host);
    std::string byte;
    int ip = 0;
    int pos = 24;

    while (std::getline(ss, byte, '.'))
    {
        ip = ip | std::atoi(byte.c_str()) << pos;
        pos -= 8;
    }

    return (ip);
}
