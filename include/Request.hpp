#pragma once

<<<<<<< Updated upstream
#include <string.h>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>
#include "parser.hpp"

struct	serverinfo
{
	Server			_rightServer;
	std::string		location;
	std::string		link;
	std::string		to_client;
};

struct	requestinfo
{
	std::string		URI;
	std::string		query;
	std::string		protocol;
	std::string		hostname;
	std::string		contentType;
	std::string		contentLength;
	std::string		connection;
	std::string		formatAccepted;
	std::string		body;
	int				method;
	int				_headerEnd;
	int				status;
};

class Request
{
   public:
		Request(std::string request);
		~Request(void);

		int getType(void) const;
		void printInfoRequest(void);
		void checkServer(std::vector<Server> server);

	private:
		std::string findType(std::string request);
		void 		findPort(std::string hostname);
		void 		analizeRequestLine(std::string requestLine);
		void 		lookForLocation(std::string location);
		void 		rightFormatLocation(void);
		void 		checkInvalidCharacters(std::string to_check);
		void 		checkOnLocation(void);
		void		bodyLength(void);

		requestinfo requestInfo;
		serverinfo	serverInfo;
};



std::string findInfo(std::string request, std::string toFind);
bool checkBody(std::string request);
=======
>>>>>>> Stashed changes
