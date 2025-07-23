#include "Response.hpp"

Response::Response(void)
{
}

Response::~Response(void)
{
}

std::string& Response::getCompleteResponse(void)
{
	_completeResponse =
	    "HTTP/1.1 200 OK\r\n"
	    "Content-Type: text/html\r\n"
	    "Content-Length: 11\r\n"
	    "\r\n"
	    "Hello World";
	return (_completeResponse);
}

std::string Response::getStatusMessage(int status_code)
{
	switch (status_code)
	{
		case 200:
			return ("OK");
		case 201:
			return ("Created");
		case 301:
			return ("Moved Permanently");
		case 302:
			return ("Found");
		case 400:
			return ("Bad Request");
		case 403:
			return ("Forbidden");
		case 404:
			return ("Not Found");
		case 405:
			return ("Method Not Allowed");
		case 413:
			return ("Payload Too Large");
		case 500:
			return ("Internal Server Error");
		case 501:
			return ("Not Implemented");
		case 502:
			return ("Bad Gateway");
		default:
			return ("Unknown");
	}
}

std::string Response::getMimeType(const std::string& filename)
{
	size_t dot_pos = filename.find_last_of('.');
	if (dot_pos == std::string::npos)
		return ("application/octet-stream");

	std::string extension = filename.substr(dot_pos + 1);

	if (extension == "html" || extension == "htm")
		return ("text/html");
	if (extension == "css")
		return ("text/css");
	if (extension == "js")
		return ("application/javascript");
	if (extension == "json")
		return ("application/json");
	if (extension == "png")
		return ("image/png");
	if (extension == "jpg" || extension == "jpeg")
		return ("image/jpeg");
	if (extension == "gif")
		return ("image/gif");
	if (extension == "txt")
		return ("text/plain");
	if (extension == "pdf")
		return ("application/pdf");

	return ("application/octet-stream");
}

std::string Response::formatHeaders(int status_code, const std::string& content_type, size_t content_length,
                                    const std::string& location)
{
	std::ostringstream headers;
	headers << "HTTP/1.1 " << status_code << " " << getStatusMessage(status_code) << "\r\n";
	headers << "Server: webserv/1.0\r\n";
	headers << "Content-Type: " << content_type << "\r\n";
	headers << "Content-Length: " << content_length << "\r\n";

	if (!location.empty())
		headers << "Location: " << location << "\r\n";

	headers << "Connection: close\r\n";
	headers << "\r\n";

	return (headers.str());
}
