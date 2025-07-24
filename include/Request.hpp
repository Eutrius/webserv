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

		std::string getType(void);
		void printInfoRequest(void);
		void checkServer(std::vector<Server> server);

	private:
		void findType(std::string request);
		void findPort(std::string hostname);
		void analizeRequestLine(std::string requestLine);
		void lookForLocation(std::string location);
		void rightFormatLocation(void);
		void checkInvalidCharacters(std::string to_check);
		void checkOnLocation(void);
		void bodyLength(void);

		std::map<std::string, std::string> info;
		Server				_rightServer;
		int					_headerEnd;
		int					status;
};

std::string findInfo(std::string request, std::string toFind);
bool checkBody(std::string request);