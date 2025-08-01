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
			_epoll.addFd(it->getFd(), EPOLLIN);
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

void Controller::newCGIConnection(int fd, int targetFd, int event)
{
	_epoll.addFd(fd, event);
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
			_epoll.addFd(clientFd, EPOLLIN);
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

void Controller::modifyConnection(int fd, int event)
{
	try
	{
		_epoll.modifyFd(fd, event);
	}
	catch (std::exception &e)
	{
		closeConnection(fd);
		std::cerr << e.what() << std::endl;
	}
}

int Controller::read(int fd)
{
	char buffer[BUFFER_SIZE];
	Connection &curr = _connections[fd];
	int bytesRead = ::read(fd, buffer, BUFFER_SIZE);
	if (bytesRead > 0)
	{
		curr.readBuffer.append(buffer, bytesRead);
		curr.lastActivity = time(NULL);
	}
	return (bytesRead);
}

int Controller::write(int fd)
{
	Connection &curr = _connections[fd];
	int bytesSent = ::write(fd, curr.writeBuffer.c_str() + curr.sent, curr.writeBuffer.length() - curr.sent);
	if (bytesSent > 0)
	{
		curr.sent += bytesSent;
		curr.lastActivity = time(NULL);
	}

	return (bytesSent);
}

Connection &Controller::getConnection(int fd)
{
	return (_connections[fd]);
}

con_type Controller::getConnectionTypeByFd(int fd)
{
	return (_connections[fd].type);
}

Response &Controller::getResponseByFd(int fd)
{
	return (_connections[fd].res);
}

Request &Controller::getRequestByFd(int fd)
{
	return (_connections[fd].req);
}

void Controller::handleCGIInput(int fd)
{
	Connection &curr = _connections[fd];
	Connection &target = getConnection(curr.targetFd);
	Response &res = getResponseByFd(curr.targetFd);
	Request &req = getRequestByFd(curr.targetFd);

	res.setBody(curr.readBuffer);
	res.generateHeader(200, req.getServerInfo().link, req.getServerInfo().location);
	target.writeBuffer = res.getCompleteResponse();
	modifyConnection(curr.targetFd, EPOLLOUT);
	closeConnection(fd);
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
		int cgiFD = handleCGI(server, request);
		try
		{
			newCGIConnection(cgiFD, fd, EPOLLIN);
			_epoll.modifyFd(fd, 0);
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

void Controller::generateCGIEnv(std::vector<char *> envp, serverInfo &server, requestInfo &request)
{
	std::vector<std::string> envStrings;

	envStrings.push_back(std::string("REQUEST_METHOD=") + (request.method == GET ? "GET" : "POST"));
	if (request.method == POST)
	{
		if (!request.contentType.empty())
			envStrings.push_back("CONTENT_TYPE=" + request.contentType);
		if (!request.contentLength.empty())
			envStrings.push_back("CONTENT_LENGTH=" + request.contentLength);
	}
	envStrings.push_back("QUERY_STRING=" + request.query);
	envStrings.push_back("SCRIPT_NAME=" + server.link);
	if (!request.cgiPath.empty())
		envStrings.push_back("PATH_INFO=" + request.cgiPath);
	envStrings.push_back("SERVER_NAME=" + request.hostname);
	envStrings.push_back("SERVER_PROTOCOL=" + request.protocol);
	envStrings.push_back("REQUEST_URI=" + request.URI);
	// envStrings.push_back("REMOVE_ADDR=" + );
	// envStrings.push_back("SERVER_PORT=" + );

	if (!request.formatAccepted.empty())
		envStrings.push_back("HTTP_ACCEPT=" + request.formatAccepted);

	if (!request.hostname.empty())
		envStrings.push_back("HTTP_HOST=" + request.hostname);

	if (!request.cookie.empty())
		envStrings.push_back("HTTP_COOKIE=" + request.cookie);

	for (size_t i = 0; i < request._env.size(); i++)
		envStrings.push_back(normalizeEnvName(request._env[i].first) + request._env[i].second);

	for (size_t i = 0; i < envStrings.size(); i++)
		envp.push_back(const_cast<char *>(envStrings[i].c_str()));
	envp.push_back(NULL);
}

int Controller::handleCGI(serverInfo &server, requestInfo &request)
{
	int outPipe[2];
	int inPipe[2];

	if (pipe(outPipe) == -1)
	{
		request.status = 500;
		return (-1);
	}

	if (pipe(inPipe) == -1)
	{
		close(outPipe[0]);
		close(outPipe[1]);
		request.status = 500;
		return (-1);
	}

	std::vector<char *> envp;
	generateCGIEnv(envp, server, request);

	pid_t pid = fork();
	if (pid == -1)
	{
		close(outPipe[0]);
		close(outPipe[1]);
		close(inPipe[0]);
		close(inPipe[1]);
		request.status = 500;
		return (-1);
	}

	if (pid == 0)
	{
		close(outPipe[0]);
		close(inPipe[1]);

		if (dup2(outPipe[1], STDOUT_FILENO) == -1)
			std::exit(1);
		close(outPipe[1]);

		if (dup2(STDOUT_FILENO, STDERR_FILENO) == -1)
			std::exit(1);

		if (dup2(inPipe[0], STDIN_FILENO) == -1)
			std::exit(1);
		close(inPipe[0]);

		std::vector<char *> argv;
		std::string scriptPath = server.link;
		std::string binary;

		size_t dotPos = scriptPath.find_last_of('.');
		std::string extension = scriptPath.substr(dotPos);
		binary = server._rightServer.cgi_extension[extension];

		argv.push_back(const_cast<char *>(binary.c_str()));
		argv.push_back(const_cast<char *>(scriptPath.c_str()));
		argv.push_back(NULL);

		std::string scriptDir = scriptPath.substr(0, scriptPath.find_last_of('/'));
		if (!scriptDir.empty())
			chdir(scriptDir.c_str());

		execve(binary.c_str(), &argv[0], &envp[0]);
		std::exit(1);
	}
	else
	{
		close(outPipe[1]);
		close(inPipe[0]);

		if (request.method == POST && !request.body.empty())
		{
			ssize_t bytesSent = ::write(inPipe[1], request.body.c_str(), request.body.length());
			if (bytesSent == -1)
			{
				close(outPipe[0]);
				close(inPipe[1]);
				request.status = 500;
				return (-1);
			}
		}
		close(inPipe[1]);
		return (outPipe[0]);
	}
}

std::string Controller::normalizeEnvName(std::string headerName)
{
	std::string env = "HTTP_";
	for (std::string::const_iterator it = headerName.begin(); it != headerName.end(); ++it)
	{
		if (*it == '-')
			env += '_';
		else
			env += std::toupper(static_cast<unsigned char>(*it));
	}
	env += "=";
	return (env);
}
