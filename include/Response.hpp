#pragma once

#include <sstream>
#include <string>
#include "Request.hpp"

class Response
{
   public:
	Response(void);
	Response(Request req);
	~Response(void);
	std::string getCompleteResponse(void);

   private:
	void handleRedirect(Request req);
	std::string handleError(Request req);
	std::string getStatusMessage(int status_code);
	std::string getMimeType(const std::string& filename);
	std::string formatHeaders(int status_code, const std::string& content_type, size_t content_length,
	                          const std::string& location);

	std::string _body;
	std::string _header;
};
