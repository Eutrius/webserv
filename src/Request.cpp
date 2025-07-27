#include "Request.hpp"

Request::Request(std::string request)
{
	std::string adress;
	_requestInfo.status = 200;
	std::string requestLine = request.substr(0, request.find("\n"));

	try
	{
		analizeRequestLine(requestLine);
		adress = findInfo(request, "Host:");
		if (adress == "")
		{
			_requestInfo.status = 400;
			throw std::runtime_error("Bad request: Host not found\n");
		}
		findPort(adress);
		_requestInfo.connection = findInfo(request, "Connection:");
		_requestInfo.formatAccepted = findInfo(request, "Accept:");
		_requestInfo.contentLength = findInfo(request, "Content-Length");
		_requestInfo._headerEnd = request.find("\r\n\r\n");
		_requestInfo.body = request.substr(_requestInfo._headerEnd + 4);
		_requestInfo.contentType = findInfo(request, "Content-Type");
	}
	catch (std::exception &error)
	{
		std::cout << error.what() << std::endl;
	}
}

Request::~Request(void)
{
}

int Request::getType(void) const
{
	return (_requestInfo.method);
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

void Request::findPort(std::string adress)
{
	int pos;
	std::string port;

	pos = adress.find(":");
	if (pos == -1)
		_requestInfo.hostname = adress;
}

void Request::bodyLength(void)
{
	if (std::atoi(_requestInfo.contentLength.c_str()) != (int) _requestInfo.body.length())
	{
		_requestInfo.status = 400;
		throw std::runtime_error("Bad request: Invalid body Lenght\n");
	}
	if (_requestInfo._headerEnd == -1)
	{
		_requestInfo.status = 400;
		throw std::runtime_error("Bad request: no end of file\n");
	}
}
void Request::analizeRequestLine(std::string requestLine)
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

void Request::rightFormatLocation(void)
{
	size_t pos;

	if (_requestInfo.URI[0] != '/')
	{
		_requestInfo.status = 400;
		throw std::runtime_error("Bad request: no slash in URI\n");
	}
	pos = _requestInfo.URI.find("%20");
	while (pos != std::string::npos)
	{
		_requestInfo.URI.replace(pos, 3, " ");
		pos = _requestInfo.URI.find("%20");
	}
	pos = _requestInfo.URI.find("?");
	if (pos == std::string::npos)
		_requestInfo.query = "";
	else
	{
		_requestInfo.query = _requestInfo.URI.substr(pos);
		_requestInfo.URI.erase(pos);
	}
}

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

void Request::checkServer(std::vector<Server> server)
{
	for (int i = server.size() - 1; i >= 0; i--)
	{
		Server it = server[i];
		_serverInfo._rightServer = it;
		if (std::find(it.server_name.begin(), it.server_name.end(), _requestInfo.hostname) != it.server_name.end())
			return;
	}
	lookForLocation(_requestInfo.URI);
	checkOnLocation();
}

void Request::lookForLocation(std::string location)
{
	std::string temp;
	temp = location;
	std::map<std::string, Location>::iterator it;

	for (it = _serverInfo._rightServer.location.begin(); it != _serverInfo._rightServer.location.end(); it++)
	{
		std::cout << it->first << std::endl;
		if (location.find(it->first) != std::string::npos && it->first.length() > _serverInfo.location.length())
			_serverInfo.location = it->first;
	}
}

void Request::checkOnLocation(void)
{
	int pos;
	// struct stat data;

	if (_serverInfo._rightServer.location[_serverInfo.location].return_path.second != "")
	{
		_requestInfo.status = 301;
		_serverInfo.to_client = _serverInfo._rightServer.location[_serverInfo.location].return_path.second;
		return;
	}
	pos = _requestInfo.URI.find(_serverInfo.location);
	_serverInfo.link = _requestInfo.URI;
	_serverInfo.link.insert(pos, _serverInfo._rightServer.location[_serverInfo.location].root);
	pos = _serverInfo.link.find(_serverInfo.location);
	// serverInfo.link.replace(pos, serverInfo.location.length(), "");
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
	// stat(info["link"].c_str(), &data);
	// if (data.st_mode & S_IFREG)
	// {
	// 	info["file"] = info["link"].substr(info["link"].rfind("\\"));
	// 	info["link"].erase(info["link"].rfind("\\"));
	// }
	// if (data->st_mode & S_IFDIR  && (data->st_mode))
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
	std::cout << "STATUS: " << _requestInfo.status << std::endl << std::endl;
	// printServers(temp);
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
