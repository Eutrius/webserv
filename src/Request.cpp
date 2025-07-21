#include "Request.hpp"

Request::Request(void)
{}

Request::Request(std::string request)
{

	std::string	adress;
	std::string requestLine = request.substr(0, request.find("\n"));

	analizeRequestLine(requestLine);
	adress = this->findInfo(request, "Host:");
	if (adress == "")
		std::cout << "Bad Request" << std::endl;
	findPort(adress);
	_connection = this->findInfo(request, "Connection:");
	_accept = this->findInfo(request, "Accept:");
	_bodyLength = findInfo(request, "Content-Length");
	_headerEnd = request.find("\r\n\r\n");
	_body = request.substr(_headerEnd);
	if (_bodyLength != "" && std::stoi(_bodyLength) != _body.length())
		std::cout << "Invalid body Lenght" << std::endl;
	if (_headerEnd == -1)
		std::cout << "Bad request" << std::endl;
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
	std::cout << "Invalid Method" << std::endl;
}

std::string	Request::findInfo(std::string request, std::string toFind)
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

void	Request::findPort(std::string adress)
{
	int pos;
	std::string port;
	int num;

	pos = adress.find(":");
	if (pos == -1)
	{
		_hostname = adress;
		_port.first = atoi_ip("localhost");
		_port.second = 8080;
		return ;
	}
	port.append(adress, pos + 1);
	adress.erase(pos);
	if (adress == "localhost")
		adress = "127.0.0.1";
    _port.first = atoi_ip(adress);
	_port.second = std::stoi(port);

}

void	Request::analizeRequestLine(std::string requestLine)
{
	std::string	protocol;
	std::string check;

	this->findType(requestLine);
	_location = this->findInfo(requestLine, _type);
	protocol = this->findInfo(requestLine, _location);
	check = this->findInfo(requestLine, protocol);
	if (_location == "" || protocol == "" || check != "")
		std::cout << "Bad request" << std::endl;
	if (protocol != "HTTP/1.1")
		std::cout << "Invalid HTTP Protocol" << std::endl;

}

void	Request::checkServer(std::vector<Server> server)
{
	for (int i = server.size() - 1; i >= 0; i--)
	{
		Server it=server[i];
		if (std::find(it.listen.begin(), it.listen.end(), _port) != it.listen.end())
		{
			_rightServer = it;
			if (std::find(it.server_name.begin(), it.server_name.end(), _hostname) != it.server_name.end())
				return ;
		}
	}
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
	printServers(temp);
}
