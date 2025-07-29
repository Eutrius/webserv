#include "Controller.hpp"

Controller::Controller(Epoll &epoll) : _epoll(epoll)
{
}

Controller::~Controller(void)
{
	std::map<int, Connection>::iterator it = _connections.begin();
	while (it != _connections.end())
	{
		std::map<int, Connection>::iterator curr = it++;
		closeConnection(curr->first);
	}
}

int Controller::initServers(std::vector<Socket> &sockets)
{
	std::vector<Socket>::iterator it = sockets.begin();
	while (it != sockets.end())
	{
		try
		{
			_epoll.addFd(it->getFd());
			newServerConnection(*it);
			it++;
		}
		catch (std::exception &e)
		{
			std::cout << e.what() << std::endl;
			it = sockets.erase(it);
		}
	}

	if (sockets.empty())
		return (1);
	return (0);
}

void Controller::newServerConnection(Socket socket)
{
	Connection curr;
	curr.type = CON_SERVER;
	curr.socket = socket;
	curr.servers = socket.getServers();
	_connections[socket.getFd()] = curr;
}

void Controller::newCGIConnection(int fd, int targetFd)
{
	Connection curr;
	curr.type = CON_CGI;
	curr.targetFd = targetFd;
	_connections[fd] = curr;
}

void Controller::newClientConnection(int fd)
{
	Connection &server = _connections[fd];
	int clientFd = server.socket.accept();
	if (clientFd > 0)
	{
		try
		{
			_epoll.addFd(clientFd);
			Connection conn;
			conn.type = CON_CLIENT;
			conn.sent = 0;
			conn.isWaiting = false;
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
	_epoll.removeFd(fd);
	_connections.erase(fd);
	close(fd);
}

int Controller::read(int fd)
{
	char buffer[BUFFER_SIZE];
	Connection &curr = _connections[fd];
	int bytes_read = ::read(fd, buffer, BUFFER_SIZE);
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
	int bytes_sent = ::write(fd, curr.writeBuffer.c_str() + curr.sent, curr.writeBuffer.length() - curr.sent);
	if (bytes_sent > 0)
	{
		curr.sent += bytes_sent;
		curr.lastActivity = time(NULL);
	}
	if (curr.sent >= curr.writeBuffer.length())
		return (1);

	return (0);
}

Connection &Controller::getConnection(int fd)
{
	return (_connections[fd]);
}

con_type Controller::getConnectionTypeByFd(int fd)
{
	return (_connections[fd].type);
}

int Controller::handleRequest(int fd)
{
	Connection &curr = _connections[fd];

	Request req(curr.readBuffer, curr.servers);
	req.printInfoRequest();
	curr.req = req;

	serverInfo &server = curr.req.getServerInfo();
	requestInfo &request = curr.req.getInfo();
	Location location = server._rightServer.location[server.location];

	Response res;

	if (request.isRedirect)
	{
		if (!server.to_client.empty())
		{
			res.handleRedirect(server, request);
			curr.writeBuffer = res.getCompleteResponse();
			return (1);
		}
	}
	if (request.isCGI)
	{
		int cgiFD = res.handleCGI(server, request, location);
		if (cgiFD != -1)
		{
			try
			{
				_epoll.addFd(cgiFD);
				_epoll.modifyFd(fd, 0);
				newCGIConnection(cgiFD, fd);
				curr.res = res;
				return (0);
			}
			catch (std::exception &e)
			{
				request.status = 500;
				closeConnection(cgiFD);
				std::cerr << e.what() << std::endl;
			}
		}
	}
	else if (request.status == 200)
	{
		if (request.method == GET)
		{
			if (res.handleGet(server, request, location))
			{
				curr.writeBuffer = res.getCompleteResponse();
				return (1);
			}
		}
		else if (request.method == POST)
		{
			if (res.handlePost(request, location))
			{
				curr.writeBuffer = res.getCompleteResponse();
				return (1);
			}
		}
		else if (request.method == DELETE)
			res.handleDelete(server, request);
	}

	if (res.handleError(server, request, location))
	{
		curr.writeBuffer = res.getCompleteResponse();
		return (1);
	}

	res.defaultHtmlBody(request.status);
	curr.writeBuffer = res.getCompleteResponse();
	return (1);
}
