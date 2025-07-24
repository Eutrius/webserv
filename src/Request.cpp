#include "Request.hpp"

Request::Request(std::string request) : status(200)
{
	std::string adress;
	std::string requestLine = request.substr(0, request.find("\n"));

	try
	{
		analizeRequestLine(requestLine);
		adress = findInfo(request, "Host:");
		if (adress == "")
		{
			status = 400;
			throw std::runtime_error("Bad request: Host not found\n");
		}
		findPort(adress);
		info["Connection"] = findInfo(request, "Connection:");
		info["Accept"] = findInfo(request, "Accept:");
		info["Content-Length"] = findInfo(request, "Content-Length");
		_headerEnd = request.find("\r\n\r\n");
		info["body"] = request.substr(_headerEnd + 4);
		info["Content-Type"] = findInfo(request, "Content-Type");
	}
	catch (std::exception &error)
	{
		std::cout << error.what() << std::endl;
	}
}

Request::~Request(void)
{
}

std::string Request::getType(void)
{
	return (info["method"]);
}

void Request::findType(std::string request)
{
	int pos;

	pos = request.find("GET");
	if (pos >= 0)
	{
		info["method"] = "GET";
		return;
	}
	pos = request.find("POST");
	if (pos >= 0)
	{
		info["method"] = "POST";
		return;
	}
	pos = request.find("DELETE");
	if (pos >= 0)
	{
		info["method"] = "DELETE";
		return;
	}
	status = 405;
	throw std::runtime_error("Method not allowed\n");
}

void Request::findPort(std::string adress)
{
	int pos;
	std::string port;

	pos = adress.find(":");
	if (pos == -1)
		info["hostname"] = adress;
}

void	Request::bodyLength(void)
{
	if (std::atoi(info["Content-Length"].c_str()) != (int) info["body"].length())
		{
			status = 400;
			throw std::runtime_error("Bad request: Invalid body Lenght\n");
		}
		if (_headerEnd == -1)
		{
			status = 400;
			throw std::runtime_error("Bad request: no end of file\n");
		}

}
void Request::analizeRequestLine(std::string requestLine)
{
	std::string protocol;
	std::string check;

	checkInvalidCharacters(requestLine);
	this->findType(requestLine);
	info["URI"] = findInfo(requestLine, info["method"]);
	rightFormatLocation();
	info["protocol"] = findInfo(requestLine, info["URI"]);
	if (info["protocol"] != "HTTP/1.1")
	{
		status = 505;
		throw std::runtime_error("Invalid HTTP Protocol\n");
	}
	check = findInfo(requestLine, info["protocol"]);
	if (info["URI"] == "" || info["protocol"] == "" || check != "")
	{
		status = 400;
		throw std::runtime_error("Bad request: invalid request line\n");
	}
}

void Request::rightFormatLocation(void)
{
	size_t pos;

	if (info["URI"][0] != '/')
	{
		status = 400;
		throw std::runtime_error("Bad request: no slash in URI\n");
	}
	pos = info["URI"].find("%20");
	while (pos != std::string::npos)
	{
		info["URI"].replace(pos, 3, " ");
		pos = info["URI"].find("%20");
	}
	pos = info["URI"].find("?");
	if (pos == std::string::npos)
		info["query"] = "";
	else
	{
		info["query"] = info["URI"].substr(pos);
		info["URI"].erase(pos);
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
			status = 400;
			throw std::runtime_error("Bad request: invalid character found\n");
		}
	}
}

void Request::checkServer(std::vector<Server> server)
{
	for (int i = server.size() - 1; i >= 0; i--)
	{
		Server it = server[i];
		_rightServer = it;
		if (std::find(it.server_name.begin(), it.server_name.end(), info["hostname"]) != it.server_name.end())
				return;
	}
	lookForLocation(info["URI"]);
	checkOnLocation();
}

void Request::lookForLocation(std::string location)
{
	std::string temp;
	int pos;

	temp = ft_trim(location);
	if (_rightServer.location.find(temp) != _rightServer.location.end())
	{
		info["location"] = temp;
		return;
	}
	temp.erase(temp.length() - 1);
	pos = temp.rfind("/");
	if (pos == -1)
	{
		status = 404;
		throw std::runtime_error("Location not found\n");
	}
	else
		lookForLocation(temp.substr(0, pos));
}

void Request::checkOnLocation(void)
{
	int pos;

	pos = info["URI"].find(info["location"]);
	info["link"] = info["URI"];
	info["link"].insert(pos, _rightServer.location[info["location"]].root);
	pos = info["link"].find(info["location"]);
	info["link"].replace(pos + 1, info["location"].length(), "");
	if (std::atoi(info["Content-Length"].c_str()) > (int)_rightServer.client_max_body_size)
	{
		status = 400;
		throw std::runtime_error("Content length exceeds client max body size\n");
	}
	if ((!(_rightServer.location[info["location"]].methods & 1) && this->getType() == "GET") ||
	(!(_rightServer.location[info["location"]].methods & 2) && this->getType() == "POST") ||
	(!(_rightServer.location[info["location"]].methods & 4) && this->getType() == "DELETE"))
	{
		status = 405;
		throw std::runtime_error("Method not Allowed\n");
	}


}

void Request::printInfoRequest(void)
{
	std::vector<Server> temp;

	temp.push_back(_rightServer);
	std::cout << "TYPE: " << this->info["method"] << std::endl;
	std::cout << "URI: " << info["URI"] << std::endl;
	std::cout << "LOCATION: " << info["location"] << std::endl;
	std::cout << "link: " << info["link"] << std::endl;
	std::cout << "HOSTNAME: " << info["hostname"] << std::endl;
	std::cout << "CONNECTION: " << info["Connection"] << std::endl;
	std::cout << "FILE ACCEPTED: " << info["Accept"] << std::endl;
	std::cout << "BODY LENGTH: " << info["Content-Length"] << std::endl;
	std::cout << "CONTENT TYPE: " << info["Content-Type"] << std::endl;
	std::cout << "QUERY: " << info["query"] << std::endl;
	std::cout << "STATUS: " << status << std::endl << std::endl;
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
