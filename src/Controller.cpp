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

	std::map<int, time_t>::iterator i;

	for (i = _cgiConnections.begin(); i != _cgiConnections.end(); i++)
	{
		if (std::time(NULL) - i->second > CGI_TIMEOUT)
		{
			int status;
			kill(i->first, SIGKILL);
			waitpid(i->first, &status, 0);
		}
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
	Connection conn;
	conn.type = CON_SERVER;
	conn.socket = socket;
	_connections[socket.getFd()] = conn;
}

void Controller::newCGIConnection(int fd, int targetFd, int event)
{
	_epoll.addFd(fd, event);
	Connection conn;
	conn.type = CON_CGI;
	conn.lastActivity = std::time(NULL);
	conn.targetFd = targetFd;
	_connections[fd] = conn;
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
			conn.socket = server.socket;
			conn.lastActivity = std::time(NULL);
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

void Controller::checkTimeouts(void)
{
	std::map<int, Connection>::iterator it;
	for (it = _connections.begin(); it != _connections.end(); it++)
	{
		Connection &curr = it->second;

		if (curr.type & CON_SERVER)
			continue;

		serverInfo &server = curr.req.getServerInfo();
		requestInfo &request = curr.req.getInfo();
		Location location = server._rightServer.location[server.location];
		Response &res = curr.res;

		if (curr.type & CON_CLIENT)
		{
			if (std::time(NULL) - curr.lastActivity > (request.isCGI ? CGI_TIMEOUT : TIMEOUT))
			{
				request.status = 408;
				res.setBody("");
				res.handleError(server, request, location);
				curr.writeBuffer = res.getCompleteResponse();
				modifyConnection(it->first, EPOLLOUT);
			}
		}

		if (curr.type & CON_CGI)
		{
			if (std::time(NULL) - curr.lastActivity > CGI_TIMEOUT + TIMEOUT)
				closeConnection(it->first);
		}
	}

	std::map<int, time_t>::iterator i;
	for (i = _cgiConnections.begin(); i != _cgiConnections.end(); i++)
	{
		if (std::time(NULL) - i->second > CGI_TIMEOUT)
			kill(i->first, SIGKILL);
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
		curr.lastActivity = std::time(NULL);
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
		curr.lastActivity = std::time(NULL);
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

bool Controller::isValidConnection(int fd)
{
	return (_connections.find(fd) != _connections.end());
}

int Controller::handleRequest(int fd, std::vector<std::string> cookie)
{
	Connection &curr = _connections[fd];

	Request req(curr.readBuffer, curr.socket.getServers());
	curr.req = req;

	(void) cookie;
	serverInfo &server = curr.req.getServerInfo();
	requestInfo &request = curr.req.getInfo();
	Location location = server._rightServer.location[server.location];
	Response &res = curr.res;

	for (size_t i = 0; i < cookie.size(); i++)
		if (!cookie[i].empty())
			res.appendHeader("Set-Cookie: " + cookie[i]);

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
		if (handleCGI(fd))
			return (0);
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
		{
			if (res.handleDelete(server, request))
			{
				curr.writeBuffer = res.getCompleteResponse();
				return (1);
			}
		}
	}

	res.handleError(server, request, location);
	curr.writeBuffer = res.getCompleteResponse();
	return (1);
}

int Controller::handleCGI(int fd)
{
	Connection &curr = _connections[fd];
	serverInfo &server = curr.req.getServerInfo();
	requestInfo &request = curr.req.getInfo();
	Location location = server._rightServer.location[server.location];

	std::string fullPath = server.link;
	std::string extension = fullPath.substr(fullPath.find_last_of('.'));
	std::string binary = location.cgi_extension[extension];

	if (binary.empty())
	{
		request.status = 403;
		return (0);
	}

	int outPipe[2];
	int inPipe[2];

	if (initPipes(inPipe, outPipe))
	{
		request.status = 500;
		return (0);
	}

	try
	{
		if (request.method == POST && !request.body.empty())
			newCGIConnection(inPipe[1], 0, EPOLLOUT);
		newCGIConnection(outPipe[0], fd, EPOLLIN);
		_epoll.modifyFd(fd, 0);
	}
	catch (std::exception &e)
	{
		closeConnection(inPipe[1]);
		closeConnection(outPipe[0]);
		close(outPipe[1]);
		close(inPipe[0]);
		request.status = 500;
		return (0);
	}

	std::vector<char *> envp;
	std::vector<std::string> envStrings;
	std::string scriptName = fullPath.substr(fullPath.find_last_of("/") + 1);
	std::string parentDir = fullPath.substr(0, fullPath.find_last_of('/'));
	generateCGIEnv(envp, envStrings, server, request, curr.socket.getHost());

	pid_t pid = fork();
	if (pid == -1)
	{
		closeConnection(inPipe[1]);
		closeConnection(outPipe[0]);
		close(outPipe[1]);
		close(inPipe[0]);
		request.status = 500;
		return (0);
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
		argv.push_back(const_cast<char *>(binary.c_str()));
		argv.push_back(const_cast<char *>(scriptName.c_str()));
		argv.push_back(NULL);

		if (!parentDir.empty())
		{
			if (chdir(parentDir.c_str()) == -1)
				std::exit(1);
		}

		execve(binary.c_str(), &argv[0], &envp[0]);
		std::exit(1);
	}
	else
	{
		close(outPipe[1]);
		close(inPipe[0]);
		_cgiConnections[pid] = std::time(NULL);

		if (request.method == POST && !request.body.empty())
		{
			Connection &cgiInput = getConnection(inPipe[1]);
			cgiInput.pid = pid;
			cgiInput.sent = 0;
			cgiInput.writeBuffer = request.body;
		}
		else
			close(inPipe[1]);

		Connection &cgiOutput = getConnection(outPipe[0]);
		cgiOutput.pid = pid;
		return (1);
	}
}

int Controller::initPipes(int inPipe[2], int outPipe[2])
{
	if (pipe(outPipe) == -1)
		return (1);

	if (pipe(inPipe) == -1)
	{
		close(outPipe[0]);
		close(outPipe[1]);
		return (1);
	}

	int outPipeFlags = fcntl(outPipe[0], F_GETFL);
	if (outPipeFlags == -1 || fcntl(outPipe[0], F_SETFL, outPipeFlags | O_NONBLOCK) == -1)
	{
		close(outPipe[0]);
		close(outPipe[1]);
		close(inPipe[0]);
		close(inPipe[1]);
		return (1);
	}

	int inPipeFlags = fcntl(inPipe[1], F_GETFL);
	if (inPipeFlags == -1 || fcntl(inPipe[1], F_SETFL, inPipeFlags | O_NONBLOCK) == -1)
	{
		close(outPipe[0]);
		close(outPipe[1]);
		close(inPipe[0]);
		close(inPipe[1]);
		return (1);
	}
	return (0);
}

void Controller::handleCGIOutput(int fd)
{
	Connection &curr = _connections[fd];
	Connection &target = getConnection(curr.targetFd);
	Response &res = getResponseByFd(curr.targetFd);
	Request &req = getRequestByFd(curr.targetFd);

	serverInfo &server = req.getServerInfo();
	requestInfo &request = req.getInfo();
	Location location = server._rightServer.location[server.location];

	int pid_status;
	waitpid(curr.pid, &pid_status, WNOHANG);
	if (WIFSIGNALED(pid_status) || (WIFEXITED(pid_status) && WEXITSTATUS(pid_status)))
	{
		if (WEXITSTATUS(pid_status))
			request.status = 500;
		else
			request.status = 408;

		_cgiConnections.erase(curr.pid);
		res.setBody("");
		res.handleError(server, request, location);
		target.writeBuffer = res.getCompleteResponse();
		modifyConnection(curr.targetFd, EPOLLOUT);
		closeConnection(fd);
		return;
	}

	std::string cgiOutput = curr.readBuffer;
	size_t headerEndPos = cgiOutput.find("\r\n\r\n");
	if (headerEndPos == std::string::npos)
	{
		headerEndPos = cgiOutput.find("\n\n");
		if (headerEndPos == std::string::npos)
			headerEndPos = 0;
		else
			headerEndPos += 2;
	}
	else
		headerEndPos += 4;

	std::string header = cgiOutput.substr(0, headerEndPos);
	std::string body = cgiOutput.substr(headerEndPos);
	std::string additionalHeaders = extractAdditionalHeaders(header);

	std::string contentType = findInfo(header, "Content-Type");
	if (contentType.empty())
		contentType = "text/html";

	std::string status = findInfo(header, "Status");
	int statusCode;
	if (status.empty())
		statusCode = 200;
	else
		statusCode = std::atoi(status.c_str());

	res.setBody(body);
	res.generateHeader(statusCode, contentType, req.getServerInfo().location);
	res.appendHeader(additionalHeaders);
	target.writeBuffer = res.getCompleteResponse();

	modifyConnection(curr.targetFd, EPOLLOUT);
	_cgiConnections.erase(curr.pid);
	closeConnection(fd);
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

void Controller::generateCGIEnv(std::vector<char *> &envp, std::vector<std::string> &envStrings, serverInfo &server,
                                requestInfo &request, t_host host)
{
	envStrings.push_back(std::string("REQUEST_METHOD=") + (request.method == GET ? "GET" : "POST"));
	envStrings.push_back("QUERY_STRING=" + request.query);
	envStrings.push_back("SERVER_NAME=" + server._rightServer.server_name[0]);
	envStrings.push_back("SERVER_PROTOCOL=" + request.protocol);
	envStrings.push_back("REQUEST_URI=" + request.URI);
	envStrings.push_back("REMOTE_ADDR=" + itoaIP(host.first));
	envStrings.push_back("REDIRECT_STATUS=CGI");

	if (request.method == POST)
	{
		if (!request.contentType.empty())
			envStrings.push_back("CONTENT_TYPE=" + request.contentType);
		else
			envStrings.push_back("CONTENT_TYPE=application/octet-stream");

		if (!request.contentLength.empty())
			envStrings.push_back("CONTENT_LENGTH=" + request.contentLength);
		else
		{
			std::stringstream contentLength;
			contentLength << request.body.size();
			envStrings.push_back("CONTENT_LENGTH=" + contentLength.str());
		}
	}

	std::stringstream port;
	port << host.second;
	envStrings.push_back("SERVER_PORT=" + port.str());

	std::string script = server.link.substr(server.link.find_last_of("/") + 1);
	envStrings.push_back("SCRIPT_FILENAME=" + script);

	if (!request.cgiPath.empty())
		envStrings.push_back("PATH_INFO=" + request.cgiPath);

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

std::string Controller::extractAdditionalHeaders(std::string header)
{
	std::istringstream headerStream(header);
	std::string headers;
	std::string line;

	while (std::getline(headerStream, line))
	{
		if (line.empty())
			continue;

		if (line[line.size() - 1] != '\r')
			line += "\r";

		if (line.find("Status:") == 0)
			continue;

		if (line.find("Content-Type:") == 0)
			continue;

		if (line.find("Content-Length:") == 0)
			continue;

		headers += line + "\n";
	}
	return (headers);
}

std::string Controller::itoaIP(int ip)
{
	std::stringstream ipStr;

	ipStr << ((ip >> 24) & 0xFF) << '.';
	ipStr << ((ip >> 16) & 0xFF) << '.';
	ipStr << ((ip >> 8) & 0xFF) << '.';
	ipStr << (ip & 0xFF);

	return (ipStr.str());
}
