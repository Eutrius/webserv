#pragma once

#include <iostream>
#include <vector>
#include <cctype>
#include <sstream>
#include <string>
#include <string.h>
#include "webserver.hpp"

class Request
{
	public:
		Request(void);
		Request(std::string request);
		virtual ~Request(void);

		std::string	getType(void) const;
		void		parsingInput(std::string request);
		void		findType(std::string request);
		std::string	findInfo(std::string request, std::string toFind);
		void		printInfoRequest(void) const;
		void		findPort(std::string hostname);
		void		checkServer(std::vector<Server> server);
		void		analizeRequestLine(std::string requestLine);
		void		lookForLocation(void);

	protected:
		Server				_rightServer;
		Location			_rightLocation;
		std::string 		_type;
		std::pair<int, int> _port;
		std::string 		_location;
		std::string			_connection;
		std::string 		_accept;
		std::string 		_hostname;
		std::string			_bodyLength;
		std::string			_body;
		int					_headerEnd;
};
