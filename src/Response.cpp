#include "Response.hpp"

Response::Response(void)
{
}

Response::Response(Request req)
{
	serverInfo server = req.getServerInfo();
	requestInfo request = req.getInfo();

	if (request.status == 301)
	{
		handleRedirect(req);
		return;
	}
	else if (request.status == 200)
	{
		return;
	}
}

Response::~Response(void)
{
}

void Response::handleRedirect(Request req)
{
	serverInfo server = req.getServerInfo();
	requestInfo request = req.getInfo();

	std::ostringstream response;
	response << "HTTP/1.1 " << request.status << " " << getStatusMessage(request.status) << "\r\n"
	         << "Location: " << server.to_client << "\r\n"
	         << "Content-Length: 0\r\n"
	         << "Connection: close\r\n"
	         << "\r\n";
	_header = response.str();
}

std::string Response::handleError(Request req)
{
	std::string body;
	(void) req;

	return (body);
}

std::string Response::getCompleteResponse(void)
{
	if (_header.empty())
	{
		_header =
		    "HTTP/1.1 200 OK\r\n"
		    "Content-Type: text/html\r\n"
		    "Content-Length: 11\r\n"
		    "\r\n";
		_body = "Hello World";
	}

	return (_header + _body);
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
