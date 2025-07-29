#pragma once

#include <dirent.h>
#include <sstream>
#include <string>
#include "Request.hpp"

class Response
{
   public:
	Response(void);
	~Response(void);
	std::string getCompleteResponse(void);
	void handleRedirect(Request req);
	void defaultHtmlBody(int statusCode);
	std::string handleError(Request req);
	std::string getStatusMessage(int statusCode);
	std::string getMimeType(std::string filename);
	void generateHeader(int statusCode, std::string contentType, std::string location);
	int generateAutoindex(std::string path, std::string uri);

	static bool fileExists(std::string path);
	static bool isDirectory(std::string path);

   private:
	std::string _body;
	std::string _header;
};
