#pragma once

#include <algorithm>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

struct Client
{
	int id;
	std::vector<std::string> info;
};

class	Cookie
{
	public:
		Cookie(void);
		Cookie(const Cookie& ref);
		Cookie& operator = (const Cookie& ref);
		~Cookie(void);

		std::vector<Client> getClients(void) const;
		void	createCookie(void);
		void	analizeCookie(std::string line);

	private:
		std::vector<Client>	clients;
		static int id;
};