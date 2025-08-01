#include "Cookie.hpp"

Cookie::Cookie(void){}

Cookie::Cookie(const Cookie& ref)
{
	*this = ref;
}

Cookie& Cookie::operator = (const Cookie& ref)
{
	this->clients = ref.clients;
	return (*this);
}

Cookie::~Cookie(void) {}

std::vector<Client> Cookie::getClients(void) const
{
	return (this->clients);
}

void	Cookie::createCookie(void)
{
	Client new_client;

	new_client.id = generateId();
	clients.push_back(new_client);
}

void	Cookie::createCookie(std::string id)
{
	Client new_client;

	new_client.id = id;
	clients.push_back(new_client);
}

void	Cookie::analizeCookie(std::string line)
{
	std::pair <std::string, std::string> value;
	int	client;

	client = findId(line);
	clients[client].info = line;
}

int		Cookie::findId(std::string line)
{
	std::string id;
	int	client = -1;
	int pos;
	int	end;

	pos = line.find("session_id=");
	if (pos != -1)
	{
		end = line.find(";", pos);
		if (pos == -1)
			end = line.find("\n", pos);
		id.append(line, pos + 11, end - pos - 12);
		for (size_t i = 0; i < clients.size(); i++)
			if (clients[i].id == id)
				client = i;
		if (client == -1)
		{
			createCookie(id);
			client = clients.size();
		}
	}
	else
		throw std::runtime_error("Bad request: No session id in Cookie");
	return (client - 1);
}

std::string	Cookie::generateId(void)
{
	std::string id;
	int		c;

	for (int i = 0; i < 8; i++)
	{
		c = (std::rand() % 97) + 33;
		id.insert(i, 1, c);
	}
	return (id);
}

void	Cookie::printClients(void)
{
	for (size_t i = 0; i < clients.size(); i ++)
	{
		std::cout << clients[i].id << std::endl;
		std::cout << clients[i].info << std::endl;
	}
}