#include "Request.hpp"

Request::Request(void)
{
}

Request::Request(std::string request, std::vector<Server> server)
{
	std::string adress;
	int curr_pos = 0;

	_requestInfo.status = 200;
	std::string line = request.substr(0, request.find("\n"));
	_requestInfo.isCGI = false;
	_requestInfo.isRedirect = false;

	try
	{
		_requestInfo._headerEnd = request.find("\r\n\r\n");
		_requestInfo.body = request.substr(_requestInfo._headerEnd + 4);
		if (_requestInfo._headerEnd == -1)
		{
			_requestInfo.status = 400;
			throw std::runtime_error("Bad request: No end of file\n");
		}
		analyzeRequestLine(line);
		curr_pos += line.length() + 1;
		request = request.substr(curr_pos);
		analyzeHeader(request.substr(0, _requestInfo._headerEnd), curr_pos);
		cleanFile();
		checkServer(server);
		if (_requestInfo.cookie.empty())
			_requestInfo.newClient = true;
		else
			_requestInfo.newClient = false;
	}
	catch (std::exception &error)
	{
		std::cout << error.what() << std::endl;
	}
}

Request::~Request(void)
{
}

Request &Request::operator=(Request &other)
{
	_requestInfo = other._requestInfo;
	_serverInfo = other._serverInfo;
	return (*this);
}

//-------------------
//		GETTERS
//-------------------

int Request::getType(void) const
{
	return (_requestInfo.method);
}

serverInfo &Request::getServerInfo(void)
{
	return (_serverInfo);
}

requestInfo &Request::getInfo(void)
{
	return (_requestInfo);
}

//-----------------------
//		REQUEST LINE
//-----------------------

void Request::analyzeRequestLine(std::string requestLine)
{
	std::string protocol;
	std::string check;

	checkInvalidCharacters(requestLine);
	check = this->findType(requestLine);
	_requestInfo.URI = findInfo(requestLine, check);
	rightFormatLocation();
	_requestInfo.protocol = findInfo(requestLine, _requestInfo.URI);
	if (_requestInfo.protocol != "HTTP/1.1")
	{
		_requestInfo.status = 505;
		throw std::runtime_error("Invalid HTTP Protocol\n");
	}
	check = findInfo(requestLine, _requestInfo.protocol);
	if (_requestInfo.URI == "" || _requestInfo.protocol == "" || check != "")
	{
		_requestInfo.status = 400;
		throw std::runtime_error("Bad request: invalid request line\n");
	}
}

std::string Request::findType(std::string request)
{
	int pos;

	pos = request.find("GET");
	if (pos >= 0)
	{
		_requestInfo.method = 1;
		return ("GET");
	}
	pos = request.find("POST");
	if (pos >= 0)
	{
		_requestInfo.method = 2;
		return ("POST");
	}
	pos = request.find("DELETE");
	if (pos >= 0)
	{
		_requestInfo.method = 4;
		return ("DELETE");
	}
	_requestInfo.status = 405;
	throw std::runtime_error("Method not allowed\n");
}

void Request::rightFormatLocation(void)
{
	size_t pos;

	if (_requestInfo.URI[0] != '/')
	{
		_requestInfo.status = 400;
		throw std::runtime_error("Bad request: no slash in URI\n");
	}
	pos = _requestInfo.URI.find("..");
	if (pos != std::string::npos)
	{
		_requestInfo.status = 400;
		throw std::runtime_error("Bad request: traversal path in URI\n");
	}
	pos = _requestInfo.URI.find("%20");
	while (pos != std::string::npos)
	{
		_requestInfo.URI.replace(pos, 3, " ");
		pos = _requestInfo.URI.find("%20");
	}
	checkQuery();
	checkCGI();
}

void Request::checkQuery(void)
{
	size_t pos;

	pos = _requestInfo.URI.find("?");
	if (pos == std::string::npos)
		_requestInfo.query = "";
	else
	{
		_requestInfo.query = _requestInfo.URI.substr(pos + 1);
		_requestInfo.URI.erase(pos);
	}
}

void Request::checkCGI(void)
{
	size_t pos;
	std::string cgi[3] = {".py", ".php", ".sh"};

	for (int i = 0; i < 3; i++)
	{
		pos = _requestInfo.URI.find(cgi[i]);
		if (pos != std::string::npos)
		{
			_requestInfo.isCGI = true;
			pos = _requestInfo.URI.find("/", pos);
			_requestInfo.cgiPath.append(_requestInfo.URI, pos);
			_requestInfo.URI.erase(pos);
			break;
		}
		else
			_requestInfo.cgiPath = "";
	}
}

//-------------------
//		HEADER
//-------------------

void Request::analyzeHeader(std::string header, int curr_pos)
{
	std::string line;
	int pos;
	std::pair<std::string, std::string> value;

	pos = header.find("Host:");
	if (pos == -1)
	{
		_requestInfo.status = 400;
		throw std::runtime_error("Bad request: Host not found\n");
	}
	while (curr_pos < _requestInfo._headerEnd)
	{
		pos = header.find("\n");
		if (pos == -1)
		{
			_requestInfo.status = 400;
			throw std::runtime_error("Bad Request\n");
		}
		line = header.substr(0, pos);
		value = parse(line);
		if (importantInfo(value, header) == false)
			_requestInfo._env.push_back(value);
		curr_pos += line.length() + 1;
		if (curr_pos < _requestInfo._headerEnd)
			header = header.substr(line.length() + 1);
	}
}

std::pair<std::string, std::string> parse(std::string line)
{
	std::pair<std::string, std::string> value;
	int begin = 0;
	int end;

	while (line[begin] == ' ' && line[begin] != '\r' && line[begin] != '\n')
		begin++;
	if (line[begin] == '\r' || line[begin] == '\n')
		throw std::runtime_error("Bad request\n");
	end = begin + 1;
	while (line[end] != ':' && line[end] != '\r' && line[end] != '\n')
		end++;
	value.first.append(line, begin, end - begin);
	begin = end;
	while ((line[begin] == ' ' || line[begin] == ':') && line[begin] != '\r' && line[begin] != '\n')
		begin++;
	while (line[end] != '\n' && line[end] != '\r')
		end++;
	value.second.append(line, begin, end - begin);
	return (value);
}

bool Request::importantInfo(std::pair<std::string, std::string> value, std::string request)
{
	if (value.first == "Connection")
	{
		checkDuplicate(_requestInfo.connection);
		_requestInfo.connection = value.second;
	}
	else if (value.first == "Accept")
	{
		checkDuplicate(_requestInfo.formatAccepted);
		_requestInfo.formatAccepted = value.second;
	}
	else if (value.first == "Content-Length")
	{
		checkDuplicate(_requestInfo.contentLength);
		_requestInfo.contentLength = value.second;
		bodyLength();
	}
	else if (value.first == "Content-Type")
	{
		checkDuplicate(_requestInfo.contentType);
		_requestInfo.contentType = parseContentType(value.second, request);
	}
	else if (value.first == "Host")
	{
		checkDuplicate(_requestInfo.hostname);
		findPort(value.second);
	}
	else if (value.first == "Cookie")
	{
		checkDuplicate(_requestInfo.cookie);
		_requestInfo.cookie = value.second;
	}
	else
		return (false);
	return (true);
}

void Request::checkDuplicate(std::string header)
{
	if (!header.empty())
	{
		_requestInfo.status = 400;
		throw std::runtime_error("Bad request: double header\n");
	}
}

std::string Request::parseContentType(std::string value, std::string line)
{
	int pos;
	int end;

	pos = line.find("boundary=");
	if (pos != -1)
	{
		end = line.find("\n", pos);
		_requestInfo.boundary.append(line, pos + 9, end - pos - 10);
		pos = value.find(";");
		return (value.substr(0, pos));
	}
	return (value);
}

void Request::findPort(std::string line)
{
	int pos;

	pos = line.find(":");
	if (pos == -1)
		_requestInfo.hostname = line;
}

//---------------------
//		BODY
//---------------------

void Request::bodyLength(void)
{
	int pos;
	int end;

	if (std::atoi(_requestInfo.contentLength.c_str()) != (int) _requestInfo.body.length())
	{
		_requestInfo.status = 400;
		throw std::runtime_error("Bad request: Invalid body Lenght\n");
	}
	pos = _requestInfo.body.find("filename=");
	if (pos != -1)
	{
		end = _requestInfo.body.find("\n", pos);
		_requestInfo.filename.append(_requestInfo.body, pos + 9, end - pos - 10);
		_requestInfo.filename = removeQuotes(_requestInfo.filename);
	}
}

void Request::cleanFile(void)
{
	int end;
	int begin;

	begin = _requestInfo.body.find("\r\n\r\n");
	if (begin != -1)
	{
		begin += 4;
		end = _requestInfo.body.rfind(_requestInfo.boundary);
		_requestInfo.body = _requestInfo.body.substr(begin, end - begin - 8);
	}
}

//------------------------
//		SERVER
//------------------------

void Request::checkServer(std::vector<Server> server)
{
	int check = 0;

	for (int i = server.size() - 1; i >= 0; i--)
	{
		Server it = server[i];
		if (std::find(it.server_name.begin(), it.server_name.end(), _requestInfo.hostname) != it.server_name.end())
		{
			check = 1;
			_serverInfo._rightServer = it;
		}
	}
	if (check == 0)
		_serverInfo._rightServer = server[0];
	lookForLocation(_requestInfo.URI);
	if (_requestInfo.status != 404)
		checkOnLocation();
}

void Request::lookForLocation(std::string location)
{
	std::string bestMatch = "";
	std::map<std::string, Location>::iterator it;

	for (it = _serverInfo._rightServer.location.begin(); it != _serverInfo._rightServer.location.end(); it++)
	{
		if (location.find(it->first) != std::string::npos && it->first.length() > _serverInfo.location.length())
			_serverInfo.location = it->first;
	}
	if (_serverInfo.location.empty())
		_requestInfo.status = 404;
}

void Request::checkOnLocation(void)
{
	int pos;
	int end;

	if (_serverInfo._rightServer.location[_serverInfo.location].return_path.first != -1)
	{
		_requestInfo.status = _serverInfo._rightServer.location[_serverInfo.location].return_path.first;
		_serverInfo.to_client = _serverInfo._rightServer.location[_serverInfo.location].return_path.second;
		_requestInfo.isRedirect = true;
		return;
	}
	pos = _requestInfo.URI.find(_serverInfo.location);
	_serverInfo.link = _serverInfo._rightServer.location[_serverInfo.location].root;
	if (_serverInfo.location == "/")
		pos -= 1;
	end = _requestInfo.URI.find("/", pos + 1);
	if (_serverInfo.link[_serverInfo.link.length() - 1] != '/')
		_serverInfo.link.append("/");
	if (end != -1)
		_serverInfo.link.append(_requestInfo.URI, end + 1);
	if (std::atoi(_requestInfo.contentLength.c_str()) > _serverInfo._rightServer.client_max_body_size)
	{
		_requestInfo.status = 400;
		throw std::runtime_error("Content length exceeds client max body size\n");
	}
	if (!(_serverInfo._rightServer.location[_serverInfo.location].methods & _requestInfo.method))
	{
		_requestInfo.status = 405;
		throw std::runtime_error("Method not Allowed\n");
	}
}

//---------------------
//		HELPERS
//---------------------

void Request::checkInvalidCharacters(std::string to_check)
{
	for (size_t i = 0; i < to_check.length(); i++)
	{
		char c = to_check[i];
		if (c == '\r' || c == '\n')
			continue;
		else if (c <= 31 || c >= 127 || c == '>' || c == '<' || c == '"')
		{
			_requestInfo.status = 400;
			throw std::runtime_error("Bad request: invalid character found\n");
		}
	}
}

void Request::printInfoRequest(void)
{
	std::vector<Server> temp;

	temp.push_back(_serverInfo._rightServer);
	std::cout << "TYPE: " << _requestInfo.method << std::endl;
	std::cout << "URI: " << _requestInfo.URI << std::endl;
	std::cout << "LOCATION: " << _serverInfo.location << std::endl;
	std::cout << "link: " << _serverInfo.link << std::endl;
	std::cout << "HOSTNAME: " << _requestInfo.hostname << std::endl;
	std::cout << "CONNECTION: " << _requestInfo.connection << std::endl;
	std::cout << "FILE ACCEPTED: " << _requestInfo.formatAccepted << std::endl;
	std::cout << "BODY LENGTH: " << _requestInfo.contentLength << std::endl;
	std::cout << "CONTENT TYPE: " << _requestInfo.contentType << std::endl;
	std::cout << "QUERY: " << _requestInfo.query << std::endl;
	std::cout << "FILE TO CLIENT: " << _serverInfo.to_client << std::endl;
	std::cout << "STATUS: " << _requestInfo.status << std::endl;
	std::cout << "FILENAME: " << _requestInfo.filename << std::endl;
	std::cout << "BOUNDARY: " << _requestInfo.boundary << std::endl;
	std::cout << "BODY: " << _requestInfo.body << std::endl;
	std::cout << "CGI PATH: " << _requestInfo.cgiPath << std::endl;
	printServers(temp);
}

bool checkBody(std::string request)
{
	std::string body;
	int headerEnd;
	int bodyLength;

	headerEnd = request.find("\r\n\r\n");
	if (headerEnd == -1)
		return (false);
	body = request.substr(headerEnd + 4);
	bodyLength = std::atoi(findInfo(request, "Content-Length").c_str());
	if ((int) body.length() < bodyLength)
		return (false);
	return (true);
}

std::string findInfo(std::string request, std::string toFind)
{
	int end;
	int begin;
	int pos;
	std::string result;

	pos = request.find(toFind);
	if (pos == -1)
		return ("");
	begin = pos + toFind.length();
	while (request[begin] != ' ' && request[begin] != '\r' && request[begin] != '\n')
		begin++;
	if (request[begin] == '\r' || request[begin] == '\n')
		return ("");
	end = begin + 1;
	while (request[end] != ' ' && request[end] != '\n' && request[end] != '\r')
		end++;
	result.append(request, begin + 1, end - begin - 1);
	return (result);
}

std::string removeQuotes(std::string &str)
{
	std::string result;

	if (str[str.length() - 1] == '"')
		str.erase(str.length() - 1);
	if (str[0] == '"')
		result.append(str, 1);
	return (result);
}
