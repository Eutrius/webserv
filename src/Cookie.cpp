#include "Cookie.hpp"

int id = 00;

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

	new_client.id = id;
	id += 1;
	clients.push_back(new_client);
}

void	Cookie::analizeCookie(std::string line)
{
	
}