#include "Request.hpp"

Request::Request(void)
{}

Request::Request(std::string request)
{
	int pos;
	char* format;

	pos = this->findType(request);
	_location = this->findInfo(request, _type);
	_hostname = this->findInfo(request, "Host:");
	findPort(_hostname);
	_connection = this->findInfo(request, "Connection:");
	_accept = this->findInfo(request, "Accept:");
	printInfoRequest();
}

Request::~Request(void)
{}

std::string	Request::getType(void) const
{
	return (this->_type);
}

int	Request::findType(std::string request)
{
	int pos;

	pos = request.find("GET");
	if (pos >= 0)
	{
		_type = "GET";
		return (pos);
	}
	pos = request.find("POST");
	if (pos >= 0)
	{
		_type = "POST";
		return (pos);
	}
	pos = request.find("DELETE");
	if (pos >= 0)
		_type = "DELETE";
	return (pos);
}

std::string	Request::findInfo(std::string request, std::string toFind)
{
	int	end;
	int begin;
	std::string	result = {0};

	begin = request.find(toFind) + toFind.length();
	while (request[begin] != ' ')
		begin++;
	end = begin + 1;
	while (request[end] != ' ' && request[end] != '\n')
		end++;
	result.append(request, begin + 1, end - begin - 1);
	return (result);
}

void	Request::findPort(std::string hostname)
{
	int pos;

	pos = hostname.find(":");
	_port.append(hostname, pos + 1);
	_hostname.erase(pos);
}
void	Request::printInfoRequest(void) const
{
	std::cout << "TYPE: " << this->_type << std::endl;
	std::cout << "LOCATION: " << this->_location << std::endl;
	std::cout << "HOSTNAME: " << this->_hostname << std::endl;
	std::cout << "PORT: " << this->_port << std::endl;
	std::cout << "CONNECTION: " << this->_connection << std::endl;
	std::cout << "FILE ACCEPTED: " << this->_accept << std::endl;
}