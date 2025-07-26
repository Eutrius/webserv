#include "Controller.hpp"

Controller::Controller(void)
{
}

Controller::~Controller(void)
{
}

void Controller::newConnection(int fd, std::vector<Server> servers)
{
	Connection connection;
	connection.lastActivity = time(NULL);
	connection.servers = servers;
	_connections[fd] = connection;
}

void Controller::closeConnection(int fd)
{
	_connections.erase(fd);
	close(fd);
}

int Controller::read(int fd)
{
	char buffer[BUFFER_SIZE];
	Connection &curr = _connections[fd];
	int bytes_read = recv(fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes_read > 0)
	{
		curr.request.append(buffer, bytes_read);
		curr.lastActivity = time(NULL);
		return (0);
	}
	return (1);
}

int Controller::write(int fd)
{
	Connection &curr = _connections[fd];
	int bytes_sent = send(fd, curr.response.c_str() + curr.sent, curr.response.length() - curr.sent, 0);
	if (bytes_sent > 0)
	{
		curr.sent += bytes_sent;
		curr.lastActivity = time(NULL);
	}
	return (bytes_sent <= 0 || curr.sent > curr.response.length());
}

Connection &Controller::getConnection(int fd)
{
	return (_connections[fd]);
}
