#include "Cookie.hpp"

Cookie::Cookie(void)
{
}

Cookie::Cookie(const Cookie& ref)
{
	*this = ref;
}

Cookie& Cookie::operator=(const Cookie& ref)
{
	this->clients = ref.clients;
	return (*this);
}

Cookie::~Cookie(void)
{
}

int Cookie::getCurrentClient(void) const
{
	return (this->currentClient);
}

std::vector<Client> Cookie::getClients(void) const
{
	return (this->clients);
}

void Cookie::createCookie(void)
{
	Client new_client;

	new_client.id = generateId();
	new_client.info.push_back("sessionId=" + new_client.id + "; Max-Age=20\r\n");
	clients.push_back(new_client);
	currentClient = clients.size() - 1;
}

void Cookie::createCookie(std::string id)
{
	Client new_client;

	new_client.id = id;
	new_client.info.push_back("sessionId=" + new_client.id + "; Max-Age=20\r\n");
	clients.push_back(new_client);
	currentClient = clients.size() - 1;
}

void Cookie::analizeCookie(std::string line)
{
	std::pair<std::string, std::string> value;
	int client;

	client = findId(line);
	if (client == -1)
		createCookie();
	else
	{
		currentClient = client;
		clients[currentClient].info[0] = "";
	}
}

int Cookie::findId(std::string line)
{
	std::string id;
	int client = -1;
	int pos;
	int end;

	pos = line.find("sessionId=");
	if (pos != -1)
	{
		if (pos == -1)
			end = line.find("\n", pos);
		else
			end = line.find(";", pos);
		if (end == -1)
			end = line.find(" ", pos);
		id.append(line, pos + 10, end - pos - 10);
		for (size_t i = 0; i < clients.size(); i++)
			if (clients[i].id == id)
				return (i);
		createCookie(id);
		client = clients.size();
	}
	else
		return (-1);
	return (client - 1);
}

std::string Cookie::generateId(void)
{
	std::string id;
	int c;

	for (int i = 0; i < 8; i++)
	{
		c = (std::rand() % 93) + 33;
		id.insert(i, 1, c);
	}
	return (id);
}

void Cookie::printClients(void)
{
	for (size_t i = 0; i < clients.size(); i++)
		std::cout << clients[i].id << std::endl;
}
