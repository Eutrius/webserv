#pragma once

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "Request.hpp"

struct Client
{
	std::string id;
	std::vector <std::string> info;
};

class Cookie
{
   public:
	Cookie(void);
	Cookie(const Cookie& ref);
	Cookie& operator=(const Cookie& ref);
	~Cookie(void);

	std::vector<Client> getClients(void) const;
	int getCurrentClient(void) const;
	void createCookie(void);
	void analizeCookie(std::string line);
	void printClients(void);

   private:
	int findId(std::string line);
	void createCookie(std::string id);
	std::string generateId(void);
	int	currentClient;

	std::vector<Client> clients;
};
