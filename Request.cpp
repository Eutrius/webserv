#include "Request.hpp"

Request::Request(void)
{}

Request::Request(std::string request)
{
	std::string	adress;

	this->findType(request);
	_location = this->findInfo(request, _type);
	adress = this->findInfo(request, "Host:");
	findPort(adress);
	_connection = this->findInfo(request, "Connection:");
	_accept = this->findInfo(request, "Accept:");
	_bodyLength = this->findInfo(request, "Content-Length");
	_body = request.find("\r\n\r\n");
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
	int pos;
	std::string	result;

	pos = request.find(toFind);
	if (pos == -1)
		return (0);
	begin = pos + toFind.length();
	while (request[begin] != ' ')
		begin++;
	end = begin + 1;
	while (request[end] != ' ' && request[end] != '\n')
		end++;
	result.append(request, begin + 1, end - begin - 1);
	return (result);
}

void	Request::findPort(std::string adress)
{
	int pos;
	std::string port;
	int num;

	pos = _ip.find(":");
	if (pos == -1)
	{
		_hostname = adress;
		return ;
	}
	port.append(adress, pos + 1);
	adress.erase(pos);
	if (adress == "localhost")
		adress = "127.0.0.1";
    _port.first = atoi_ip(adress);
	_port.second = std::stoi(port);

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
	std::cout << "BODY LENGTH: " << this->body_length << std::endl;
	printServers(temp);
}
