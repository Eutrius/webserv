#pragma once

#include <string.h>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "parser.hpp"

class Request
{
   public:
	Request(std::string request);
	~Request(void);

	std::string getType(void) const;
	void printInfoRequest(void) const;
	void checkServer(std::vector<Server> server);

   private:
	void findType(std::string request);
	void findPort(std::string hostname);
	void analizeRequestLine(std::string requestLine);
	void lookForLocation(std::string location);
	void rightFormatLocation(void);
	void checkInvalidCharacters(std::string to_check);
	void checkOnLocation(void);

		Server				_rightServer;
		std::string			_rightLocation;
		std::string 		_hostname;
		std::string 		_type;
		std::pair<int, int> _port;
		std::string 		_location;
		std::string			_connection;
		std::string 		_accept;
		std::string			_bodyLength;
		std::string			_body;
		int					_headerEnd;
		int					status;
};

std::string findInfo(std::string request, std::string toFind);
bool checkBody(std::string request);