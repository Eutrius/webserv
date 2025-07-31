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

	Response res;

	serverInfo &server = curr.req.getServerInfo();
	requestInfo &request = curr.req.getInfo();
	Location location = server._rightServer.location[server.location];

	if (request.isRedirect)
	{
		if (!server.to_client.empty())
		{
			res.handleRedirect(req);
			curr.writeBuffer = res.getCompleteResponse();
			return (1);
		}
	}
	else if (request.status == 200)
	{
		if (request.method == GET)
		{
			if (Response::fileExists(server.link))
			{
				if (Response::isDirectory(server.link))
				{
					if (!request.URI.empty() && request.URI[request.URI.size() - 1] != '/')
					{
						request.status = 301;
						server.to_client = request.URI + "/";
						res.handleRedirect(curr.req);
						curr.writeBuffer = res.getCompleteResponse();
						return (1);
					}

					std::string link = server.link;
					if (link[link.size() - 1] != '/')
						link += '/';

					for (size_t i = 0; i < location.index.size(); i++)
					{
						std::string path = link + location.index[i];
						if (Response::fileExists(path) && !Response::isDirectory(path))
						{
							// TODO: serve the file
							return (0);
						}
					}

					if (location.autoindex)
					{
						request.status = res.generateAutoindex(server.link, request.URI);
						if (request.status == 200)
						{
							res.generateHeader(request.status, "text/html", server.location);
							curr.writeBuffer = res.getCompleteResponse();
							return (1);
						}
					}
					else
						request.status = 403;
				}
				else
				{
					// TODO: serve the file
					return (0);
				}
			}
			else
				request.status = 404;
		}
		if (request.method == DELETE)
		{
			if (Response::fileExists(server.link))
			{
				if (unlink(server.link.c_str()) == 0)
					request.status = 204;
				else
					request.status = 403;
			}
			else
				request.status = 404;
		}
	}

	// TODO: check error page
	res.defaultHtmlBody(request.status);
	curr.writeBuffer = res.getCompleteResponse();
	return (1);
}
