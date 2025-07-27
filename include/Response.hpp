#pragma once

#include <sstream>
#include <string>
#include "Request.hpp"

class Response
{
   public:
	Response(void);
	~Response(void);
	std::string& getCompleteResponse(void);
	std::string getMimeType(const std::string& filename);
	std::string getStatusMessage(int status_code);
	std::string buildResponse(Request req);
	std::string formatHeaders(int status_code, const std::string& content_type, size_t content_length,
	                          const std::string& location);

   private:
	std::string _completeResponse;
};
