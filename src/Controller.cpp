#include "Controller.hpp"

Controller::Controller(void)
{
}

Controller::~Controller(void)
{
}

void Controller::newServerConnection(Socket socket)
{
	Connection curr;
	curr.type = CON_SERVER;
	curr.socket = socket;
	curr.servers = socket.getServers();
	_connections[socket.getFd()] = curr;
}

void Controller::newClientConnection(Epoll &epoll, int fd)
{
	Connection &server = _connections[fd];
	int clientFd = server.socket.accept();
	if (clientFd > 0)
	{
		try
		{
			epoll.addFd(clientFd);
			Connection conn;
			conn.type = CON_CLIENT;
			conn.sent = 0;
			conn.lastActivity = time(NULL);
			conn.servers = server.servers;
			_connections[clientFd] = conn;
		}
		catch (std::exception &e)
		{
			std::cout << e.what() << std::endl;
		}
	}
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
		curr.readBuffer.append(buffer, bytes_read);
		curr.lastActivity = time(NULL);
	}
	return (bytes_read);
}

int Controller::write(int fd)
{
	Connection &curr = _connections[fd];
	int bytes_sent = send(fd, curr.writeBuffer.c_str() + curr.sent, curr.writeBuffer.length() - curr.sent, 0);
	if (bytes_sent > 0)
	{
		curr.sent += bytes_sent;
		curr.lastActivity = time(NULL);
	}
	return (bytes_sent <= 0 || curr.sent > curr.writeBuffer.length());
}

Connection &Controller::getConnection(int fd)
{
	return (_connections[fd]);
}

con_type Controller::getConnectionTypeByFd(int fd)
{
	return (_connections[fd].type);
}
