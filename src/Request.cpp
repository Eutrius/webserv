#include "Request.hpp"

Request::Request(std::string request)
{
	std::string adress;
	requestInfo.status = 200;
	std::string requestLine = request.substr(0, request.find("\n"));

	try
	{
		analizeRequestLine(requestLine);
		adress = findInfo(request, "Host:");
		if (adress == "")
		{
			requestInfo.status = 400;
			throw std::runtime_error("Bad request: Host not found\n");
		}
		findPort(adress);
		requestInfo.connection = findInfo(request, "Connection:");
		requestInfo.formatAccepted = findInfo(request, "Accept:");
		requestInfo.contentLength = findInfo(request, "Content-Length");
		requestInfo._headerEnd = request.find("\r\n\r\n");
		requestInfo.body = request.substr(requestInfo._headerEnd + 4);
		requestInfo.contentType = findInfo(request, "Content-Type");
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
	return (requestInfo.method);
}

std::string Request::findType(std::string request)
{
	int pos;

	pos = request.find("GET");
	if (pos >= 0)
	{
		requestInfo.method = 1;
		return ("GET");
	}
	pos = request.find("POST");
	if (pos >= 0)
	{
		requestInfo.method = 2;
		return ("POST");
	}
	pos = request.find("DELETE");
	if (pos >= 0)
	{
		requestInfo.method = 4;
		return ("DELETE");
	}
	requestInfo.status = 405;
	throw std::runtime_error("Method not allowed\n");
}

void Request::findPort(std::string adress)
{
	int pos;
	std::string port;

	pos = adress.find(":");
	if (pos == -1)
		requestInfo.hostname = adress;
}

void	Request::bodyLength(void)
{
	if (std::atoi(requestInfo.contentLength.c_str()) != (int)requestInfo.body.length())
		{
			requestInfo.status = 400;
			throw std::runtime_error("Bad request: Invalid body Lenght\n");
		}
		if (requestInfo._headerEnd == -1)
		{
			requestInfo.status = 400;
			throw std::runtime_error("Bad request: no end of file\n");
		}

}
void Request::analizeRequestLine(std::string requestLine)
{
	std::string protocol;
	std::string check;

	checkInvalidCharacters(requestLine);
	check = this->findType(requestLine);
	requestInfo.URI = findInfo(requestLine, check);
	rightFormatLocation();
	requestInfo.protocol = findInfo(requestLine, requestInfo.URI);
	if (requestInfo.protocol != "HTTP/1.1")
	{
		requestInfo.status = 505;
		throw std::runtime_error("Invalid HTTP Protocol\n");
	}
	check = findInfo(requestLine, requestInfo.protocol);
	if (requestInfo.URI == "" || requestInfo.protocol == "" || check != "")
	{
		requestInfo.status = 400;
		throw std::runtime_error("Bad request: invalid request line\n");
	}
}

void Request::rightFormatLocation(void)
{
	size_t pos;

	if (requestInfo.URI[0] != '/')
	{
		requestInfo.status = 400;
		throw std::runtime_error("Bad request: no slash in URI\n");
	}
	pos = requestInfo.URI.find("%20");
	while (pos != std::string::npos)
	{
		requestInfo.URI.replace(pos, 3, " ");
		pos = requestInfo.URI.find("%20");
	}
	pos = requestInfo.URI.find("?");
	if (pos == std::string::npos)
		requestInfo.query = "";
	else
	{
		requestInfo.query = requestInfo.URI.substr(pos);
		requestInfo.URI.erase(pos);
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
			requestInfo.status = 400;
			throw std::runtime_error("Bad request: invalid character found\n");
		}
	}
}

void Request::checkServer(std::vector<Server> server)
{
	for (int i = server.size() - 1; i >= 0; i--)
	{
		Server it = server[i];
		serverInfo._rightServer = it;
		if (std::find(it.server_name.begin(), it.server_name.end(), requestInfo.hostname) != it.server_name.end())
				return;
	}
	lookForLocation(requestInfo.URI);
	checkOnLocation();
}

void Request::lookForLocation(std::string location)
{
	std::string temp;
	temp = location;
	std::map<std::string, Location>::iterator it;

	for (it = serverInfo._rightServer.location.begin(); it != serverInfo._rightServer.location.end(); it++)
	{
		std::cout << it->first << std::endl;
		if (location.find(it->first) != std::string::npos && it->first.length() > serverInfo.location.length())
			serverInfo.location = it->first;
	}
}

void Request::checkOnLocation(void)
{
	int pos;
	// struct stat data;

	if (serverInfo._rightServer.location[serverInfo.location].return_path.second != "")
	{
		requestInfo.status = 301;
		serverInfo.to_client =serverInfo._rightServer.location[serverInfo.location].return_path.second;
		return ;
	}
	pos = requestInfo.URI.find(serverInfo.location);
	serverInfo.link = requestInfo.URI;
	serverInfo.link.insert(pos, serverInfo._rightServer.location[serverInfo.location].root);
	pos = serverInfo.link.find(serverInfo.location);
	//serverInfo.link.replace(pos, serverInfo.location.length(), "");
	if (std::atoi(requestInfo.contentLength.c_str()) > serverInfo._rightServer.client_max_body_size)
	{
		requestInfo.status = 400;
		throw std::runtime_error("Content length exceeds client max body size\n");
	}
	if (!(serverInfo._rightServer.location[serverInfo.location].methods & requestInfo.method))
	{
		requestInfo.status = 405;
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

	temp.push_back(serverInfo._rightServer);
	std::cout << "TYPE: " << requestInfo.method << std::endl;
	std::cout << "URI: " << requestInfo.URI << std::endl;
	std::cout << "LOCATION: " << serverInfo.location << std::endl;
	std::cout << "link: " << serverInfo.link << std::endl;
	std::cout << "HOSTNAME: " << requestInfo.hostname << std::endl;
	std::cout << "CONNECTION: " << requestInfo.connection << std::endl;
	std::cout << "FILE ACCEPTED: " << requestInfo.formatAccepted << std::endl;
	std::cout << "BODY LENGTH: " << requestInfo.contentLength << std::endl;
	std::cout << "CONTENT TYPE: " << requestInfo.contentType << std::endl;
	std::cout << "QUERY: " << requestInfo.query << std::endl;
	std::cout << "FILE TO CLIENT: " << serverInfo.to_client << std::endl;
	std::cout << "STATUS: " << requestInfo.status << std::endl << std::endl;
	//printServers(temp);
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
