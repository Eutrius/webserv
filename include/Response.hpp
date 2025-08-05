#pragma once

#include <dirent.h>
#include <fcntl.h>
#include <ctime>
#include <sstream>
#include <string>
#include "Request.hpp"

class Controller;

class Response
{
   public:
	Response(void);
	~Response(void);

	std::string getCompleteResponse(void);
	std::string getStatusMessage(int statusCode);
	std::string getMimeType(std::string filename);

	void setBody(std::string body);
	void appendHeader(std::string addtionalHeader);
	void defaultHtmlBody(int statusCode);
	void generateHeader(int statusCode, std::string contentType, std::string location);
	int generateAutoindex(std::string path, std::string uri);
	std::string generateDate(void);
	int handleFile(std::string path);
	void handleError(serverInfo &server, requestInfo &request, Location &location);
	int handlePost(requestInfo &request, Location &location);
	int handleGet(serverInfo &server, requestInfo &request, Location &location);
	int handleDelete(serverInfo &server, requestInfo &request);
	void handleRedirect(serverInfo &server, requestInfo &request);
	void handleCgi(Controller &controller);
	bool checkFile(std::string path, int &err);
	bool isDirectory(std::string path);

   private:
	std::string _body;
	std::string _header;
};
