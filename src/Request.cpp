#include "Request.hpp"

Request::Request(std::string request) : status(200)
{
	std::string	adress;
	std::string requestLine = request.substr(0, request.find("\n"));
	try
	{
		checkInvalidCharacters(request);
		analizeRequestLine(requestLine);
		adress = findInfo(request, "Host:");
		if (adress == "")
		{
			status = 400;
			throw std::runtime_error("Bad request: Host not found\n");
		}
		findPort(adress);
		_connection = findInfo(request, "Connection:");
		_accept = findInfo(request, "Accept:");
		_bodyLength = findInfo(request, "Content-Length");
		_headerEnd = request.find("\r\n\r\n");
		_body = request.substr(_headerEnd + 4);
		if (std::atoi(_bodyLength.c_str()) != (int)_body.length())
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
	catch (std::exception& error)
	{
		std::cout << error.what() << std::endl;
	}
}

Request::~Request(void)
{}

std::string	Request::getType(void) const
{
	return (this->_type);
}

void	Request::findType(std::string request)
{
	int pos;

	pos = request.find("GET");
	if (pos == 0)
	{
		_type = "GET";
		return ;
	}
	pos = request.find("POST");
	if (pos == 0)
	{
		_type = "POST";
		return ;
	}
	pos = request.find("DELETE");
	if (pos == 0)
	{
		_type = "DELETE";
		return ;
	}
	status = 405;
	throw std::runtime_error("Method not allowed\n");
}



void	Request::findPort(std::string adress)
{
	int pos;
	std::string port;

	pos = adress.find(":");
	if (pos == -1)
	{
		_hostname = adress;
		_port.first = atoi_ip("localhost");
		_port.second = 80;
		return ;
	}
	port.append(adress, pos + 1);
	adress.erase(pos);
	if (adress == "localhost")
		adress = "127.0.0.1";
    _port.first = atoi_ip(adress);
	_port.second = std::atoi(port.c_str());

}

void	Request::analizeRequestLine(std::string requestLine)
{
	std::string	protocol;
	std::string check;

	this->findType(requestLine);
	_location = findInfo(requestLine, _type);
	rightFormatLocation();
	protocol = findInfo(requestLine, _location);
	if (protocol != "HTTP/1.1")
	{
		status = 505;
		throw std::runtime_error("Invalid HTTP Protocol\n");
	}
	check = findInfo(requestLine, protocol);
	if (_location == "" || protocol == "" || check != "")
	{
		status = 400;
		throw std::runtime_error("Bad request: invalid request line\n");
	}
}

void	Request::rightFormatLocation(void)
{
	size_t	pos;

	if (_location[0] != '/')
	{
		status = 400;
		throw std::runtime_error("Bad request: no slash in URI\n");
	}
	pos = _location.find("%20");
	while (pos != std::string::npos)
	{
		_location.replace(pos, 3, " ");
		pos = _location.find("%20");
	}
}

void	Request::checkInvalidCharacters(std::string to_check)
{
	for (size_t i = 0; i < to_check.length(); i++)
	{
		char c = to_check[i];
		if (c == '\r' || c == '\n')
			continue ;
		else if (c <= 31 || c >= 127 || c == '>' || c == '<'  || c == '"')
		{
			status = 400;
			throw std::runtime_error("Bad request: invalid character found\n");
		}
	}
}

void	Request::checkServer(std::vector<Server> server)
{
	int	check = 0;

	for (int i = server.size() - 1; i >= 0; i--)
	{
		Server it=server[i];
		for (size_t x = 0; x < it.listen.size(); x++)
		{
			if (it.listen[x].second == _port.second && (it.listen[x].first == _port.first || it.listen[x].first == 0 || _port.first == 0))
			{
				_rightServer = it;
				check = 1;
				if (std::find(it.server_name.begin(), it.server_name.end(), _hostname) != it.server_name.end())
					return  ;
				else
					break ;
			}
		}
	}
	if (check == 0)
	{
		status = 404;
		throw std::runtime_error("Server not found\n");
	}
	else
		lookForLocation(_location);
}

void	Request::lookForLocation(std::string location)
{
	std::string temp;
	int			pos;

	temp = ft_trim(location);
	if (_rightServer.location.find(temp) != _rightServer.location.end())
	{
		_rightLocation = temp;
		return ;
	}
	temp.erase(temp.length() - 1);
	pos = temp.rfind("/");
	if (pos == -1)
	{
		status = 404;
		throw std::runtime_error("Location not found\n");
	}
	else
	{
		lookForLocation(temp.substr(0, pos));
		checkOnLocation();
	}
}


void	Request::checkOnLocation(void)
{
	int pos;

	pos = _location.find(_rightLocation);
	_rightDir = _location.replace(pos, _location.length(), _rightServer.location[_rightDir].root);
	std::cout << "DIR: " << _rightDir << std::endl;
}

void	Request::printInfoRequest(void) const
{
	std::vector<Server> temp;

	temp.push_back(_rightServer);
	std::cout << "TYPE: " << this->_type << std::endl;
	std::cout << "LOCATION: " << this->_location << std::endl;
	std::cout << "HOSTNAME: " << this->_hostname << std::endl;
	std::cout << "IP: " << this->_port.first << std::endl;
	std::cout << "PORT: " << this->_port.second << std::endl;
	std::cout << "CONNECTION: " << this->_connection << std::endl;
	std::cout << "FILE ACCEPTED: " << this->_accept << std::endl;
	std::cout << "BODY LENGTH: " << this->_bodyLength << std::endl;
	std::cout << "LOCATION: " << _rightLocation << std::endl;
	std::cout << "STATUS: " << status << std::endl << std::endl;
	printServers(temp);
}

bool	checkBody(std::string request)
{
	std::string	body;
	int			headerEnd;
	int			bodyLength;

	headerEnd = request.find("\r\n\r\n");
	if (headerEnd == -1)
		return false;
	body = request.substr(headerEnd + 4);
	bodyLength = std::atoi(findInfo(request, "Content-Length").c_str());
	if ((int)body.length() < bodyLength)
		return false;
	return true;
}

std::string	findInfo(std::string request, std::string toFind)
{
	int	end;
	int begin;
	int pos;
	std::string	result;

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
