#include "Socket.hpp"

Socket::Socket(void) : _fd(-1)
{
}

Socket::~Socket(void)
{
}

Socket &Socket::operator=(const Socket &other)
{
	_fd = other._fd;
	_host = other._host;
	_servers = other._servers;
	return (*this);
}

void Socket::init(t_host host, std::vector<Server> servers)
{
	struct sockaddr_in address;
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd < 0)
		throw std::runtime_error("Socket: failed to create socket");

	int opt = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
		throw std::runtime_error("Socket: failed to set socket opt");

	std::cout << host.first << host.second << _fd << std::endl;
	address.sin_family = AF_INET;
	if (host.first == 0)
		address.sin_addr.s_addr = INADDR_ANY;
	else
		address.sin_addr.s_addr = htonl(host.first);
	address.sin_port = htons(host.second);
	std::memset(address.sin_zero, '\0', sizeof(address.sin_zero));

	if (bind(_fd, (struct sockaddr *) &address, sizeof(address)) < 0)
		throw std::runtime_error("Socket: failed to bind socket");

	int flags = fcntl(_fd, F_GETFL, 0);
	if (flags < 0)
		throw std::runtime_error("Socket: failed to get socket flags");

	if (fcntl(_fd, F_SETFL, flags | O_NONBLOCK) < 0)
		throw std::runtime_error("Socket: failed to make socket non-blocking");

	if (listen(_fd, SOMAXCONN) < 0)
		throw std::runtime_error("Socket: a socket failed to listen");

	_host = host;
	_servers = servers;
}

int Socket::getFd(void) const
{
	return (_fd);
}

std::vector<Server> Socket::getServers(void) const
{
	return (_servers);
}

t_host Socket::getHost(void) const
{
	return (_host);
}

int Socket::accept(void)
{
	struct sockaddr_in clientAdress;
	int clientAdressLen = sizeof(clientAdress);
	int clientFd = ::accept(_fd, (struct sockaddr *) &clientAdress, (socklen_t *) &clientAdressLen);

	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
	{
		::close(clientFd);
		return (-1);
	}

	return (clientFd);
}

std::vector<Socket> Socket::initSockets(t_serversMap serverMap)
{
	std::vector<Socket> sockets;
	for (t_serversMap::const_iterator it = serverMap.begin(); it != serverMap.end(); it++)
	{
		try
		{
			Socket socket;
			socket.init(it->first, it->second);
			sockets.push_back(socket);
		}
		catch (const std::exception &e)
		{
			std::cout << e.what() << ": ";
			std::cout << it->first.first << ":" << it->first.second;
			std::cout << std::endl;
		}
	}
	return (sockets);
}
