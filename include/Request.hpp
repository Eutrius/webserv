#pragma once

#include <iostream>
#include <vector>
#include <cctype>
#include <sstream>
#include <string>
#include <string.h>

class Request
{
	public:
		Request(void);
		Request(std::string request);
		virtual ~Request(void);

		std::string	getType(void) const;
		void		parsingInput(std::string request);
		int			findType(std::string request);
		std::string	findInfo(std::string request, std::string toFind);
		void		printInfoRequest(void) const;
		void		findPort(std::string hostname);

	protected:
		std::string _type;
		std::string _hostname;
		std::string	_port;
		std::string _location;
		std::string	_connection;
		std::string _accept;
};