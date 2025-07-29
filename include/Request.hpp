#pragma once

#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "parser.hpp"
#include "Cookie.hpp"

struct serverInfo
{
	Server _rightServer;
	std::string location;
	std::string link;
	std::string to_client;
};

struct requestInfo
{
	std::string URI;
	std::string query;
	std::string protocol;
	std::string hostname;
	std::string contentType;
	std::string contentLength;
	std::string connection;
	std::string formatAccepted;
	std::string body;
	std::string filename;
	std::string cookie;
	std::string boundary;
	std::string cgiPath;
	std::vector<std::pair<std::string, std::string> > _env;
	int method;
	int _headerEnd;
	int status;
	bool isRedirect;
	bool isCGI;
	bool newClient;
};

class Request
{
   public:
	Request(void);
	Request(std::string request, std::vector<Server> server);
	~Request(void);

	Request &operator=(Request &other);

	int getType(void) const;
	requestInfo &getInfo(void);
	serverInfo &getServerInfo(void);

	void printInfoRequest(void);
	void checkServer(std::vector<Server> server);

   private:
	std::string findType(std::string request);
	void findPort(std::string hostname);
	void analizeRequestLine(std::string requestLine);
	void lookForLocation(std::string location);
	void rightFormatLocation(void);
	void checkInvalidCharacters(std::string to_check);
	void checkOnLocation(void);
	void bodyLength(void);
	void analizeHeader(std::string header, int curr_pos);
	bool importantInfo(std::pair<std::string, std::string> value, std::string request);
	std::string parseContentType(std::string value, std::string line);
	void cleanFile(void);
	void checkDuplicate(std::string header);
	void checkCGI(void);
	void checkQuery(void);

	requestInfo _requestInfo;
	serverInfo _serverInfo;
};

std::string findInfo(std::string request, std::string toFind);
bool checkBody(std::string request);
std::pair<std::string, std::string> parse(std::string line);
std::string removeQuotes(std::string &str);
