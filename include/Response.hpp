#pragma once

#include <dirent.h>
#include <fcntl.h>
#include <sstream>
#include <string>
#include "Request.hpp"

class Response
{
   public:
	Response(void);
	~Response(void);

	std::string getCompleteResponse(void);
	std::string getStatusMessage(int statusCode);
	std::string getMimeType(std::string filename);

	void setBody(std::string body);
	void defaultHtmlBody(int statusCode);
	void generateHeader(int statusCode, std::string contentType, std::string location);
	int generateAutoindex(std::string path, std::string uri);
	int handleFile(std::string path);
	int handleError(serverInfo &server, requestInfo &request, Location &location);
	int handlePost(requestInfo &request, Location &location);
	int handleGet(serverInfo &server, requestInfo &request, Location &location);
	int handleCGI(serverInfo &server, requestInfo &request, Location &location);
	void handleDelete(serverInfo &server, requestInfo &request);
	void handleRedirect(serverInfo &server, requestInfo &request);
	bool fileExists(std::string path);
	bool isDirectory(std::string path);

   private:
	std::string _body;
	std::string _header;
};
