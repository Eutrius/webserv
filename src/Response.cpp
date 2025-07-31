#include "Response.hpp"

Response::Response(void)
{
}

Response::~Response(void)
{
}

void Response::handleRedirect(serverInfo &server, requestInfo &request)
{
	_body = "";
	generateHeader(request.status, "", server.to_client);
}
std::vector<char *> Response::generateCGIEnv(serverInfo &server, requestInfo &request)
{
	std::vector<std::string> envStrings;

	envStrings.push_back(std::string("REQUEST_METHOD=") + (request.method == GET ? "GET" : "POST"));
	if (request.method == POST)
	{
		if (!request.contentType.empty())
		{
			envStrings.push_back("CONTENT_TYPE=" + request.contentType);
		}
		if (!request.contentLength.empty())
		{
			envStrings.push_back("CONTENT_LENGTH=" + request.contentLength);
		}
		else
		{
			std::ostringstream ss;
			ss << request.body.length();
			envStrings.push_back("CONTENT_LENGTH=" + ss.str());
		}
	}
	envStrings.push_back("QUERY_STRING=" + request.query);
	envStrings.push_back("SCRIPT_NAME=" + server.link);
	// envStrings.push_back("PATH_INFO=" + );
	envStrings.push_back("SERVER_NAME=" + request.hostname);
	// envStrings.push_back("SERVER_PORT=" + );
	envStrings.push_back("SERVER_PROTOCOL=" + request.protocol);
	// envStrings.push_back("REMOVE_ADDR=" + );
	envStrings.push_back("REQUEST_URI=" + request.URI);

	// if (!request.formatAccepted.empty())
	// {
	// 	envStrings.push_back("HTTP_USER_AGENT=");
	// }

	if (!request.formatAccepted.empty())
	{
		envStrings.push_back("HTTP_ACCEPT=" + request.formatAccepted);
	}

	if (!request.hostname.empty())
	{
		envStrings.push_back("HTTP_HOST=" + request.hostname);
	}

	if (!request.cookie.empty())
	{
		envStrings.push_back("HTTP_COOKIE=" + request.cookie);
	}

	// TODO: add the rest of header info

	std::vector<char *> envp;
	for (size_t i = 0; i < envStrings.size(); i++)
	{
		envp.push_back(const_cast<char *>(envStrings[i].c_str()));
	}
	envp.push_back(NULL);

	return (envp);
}

int Response::handleCGI(serverInfo &server, requestInfo &request)
{
	int outPipe[2];
	int inPipe[2];

	if (pipe(outPipe) == -1)
	{
		request.status = 500;
		return (-1);
	}

	if (pipe(inPipe) == -1)
	{
		close(outPipe[0]);
		close(outPipe[1]);
		request.status = 500;
		return (-1);
	}

	std::vector<char *> envp = generateCGIEnv(server, request);

	pid_t pid = fork();
	if (pid == -1)
	{
		close(outPipe[0]);
		close(outPipe[1]);
		close(inPipe[0]);
		close(inPipe[1]);
		request.status = 500;
		return (-1);
	}

	if (pid == 0)
	{
		close(outPipe[0]);
		close(inPipe[1]);

		if (dup2(outPipe[1], STDOUT_FILENO) == -1)
			std::exit(1);
		close(outPipe[1]);

		if (dup2(STDOUT_FILENO, STDERR_FILENO) == -1)
			std::exit(1);

		if (dup2(inPipe[0], STDIN_FILENO) == -1)
			std::exit(1);
		close(inPipe[0]);

		std::vector<char *> argv;

		std::string scriptPath = server.link;
		std::string binary;

		size_t dotPos = scriptPath.find_last_of('.');
		if (dotPos != std::string::npos)
		{
			std::string extension = scriptPath.substr(dotPos + 1);

			if (extension == "py")
			{
				binary = "/usr/bin/python3";
			}
			else if (extension == "php")
			{
				binary = "/usr/bin/php";
			}
			else if (extension == "sh")
			{
				binary = "/bin/sh";
			}
		}

		argv.push_back(const_cast<char *>(binary.c_str()));
		argv.push_back(const_cast<char *>(scriptPath.c_str()));
		argv.push_back(NULL);

		std::string scriptDir = scriptPath.substr(0, scriptPath.find_last_of('/'));
		if (!scriptDir.empty())
			chdir(scriptDir.c_str());

		execve(binary.c_str(), &argv[0], &envp[0]);
		std::exit(1);
	}
	else
	{
		close(outPipe[1]);
		close(inPipe[0]);

		if (request.method == POST && !request.body.empty())
		{
			ssize_t bytesSent = write(inPipe[1], request.body.c_str(), request.body.length());
			if (bytesSent == -1)
			{
				close(outPipe[0]);
				close(inPipe[1]);
				request.status = 500;
				return (-1);
			}
		}
		close(inPipe[1]);

		return (outPipe[0]);
	}

	request.status = 500;
	return (-1);
}

int Response::handleFile(std::string path)
{
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file)
		return (1);

	std::ostringstream content;
	content << file.rdbuf();
	_body = content.str();
	return (0);
}

int Response::handleError(serverInfo &server, requestInfo &request, Location &location)
{
	if (!location.error_page[request.status].empty())
	{
		if (handleFile(location.error_page[request.status]))
			request.status = 403;
		else
		{
			generateHeader(request.status, getMimeType(server.link), server.location);
			return (1);
		}
	}
	return (0);
}

int Response::handleGet(serverInfo &server, requestInfo &request, Location &location)
{
	if (fileExists(server.link))
	{
		if (isDirectory(server.link))
		{
			if (!request.URI.empty() && request.URI[request.URI.size() - 1] != '/')
			{
				request.status = 301;
				server.to_client = request.URI + "/";
				handleRedirect(server, request);
				return (1);
			}

			std::string link = server.link;
			if (link[link.size() - 1] != '/')
				link += '/';

			for (size_t i = 0; i < location.index.size(); i++)
			{
				std::string path = link + location.index[i];
				if (Response::fileExists(path) && !Response::isDirectory(path))
				{
					if (handleFile(path))
						request.status = 403;
					else
					{
						generateHeader(request.status, getMimeType(path), server.location);
						return (1);
					}
				}
			}

			if (location.autoindex)
			{
				request.status = generateAutoindex(server.link, request.URI);
				if (request.status == 200)
				{
					generateHeader(request.status, "text/html", server.location);
					return (1);
				}
			}
			else
				request.status = 403;
		}
		else
		{
			if (handleFile(server.link))
				request.status = 403;
			else
			{
				generateHeader(request.status, getMimeType(server.link), server.location);
				return (1);
			}
		}
	}
	else
		request.status = 404;
	return (0);
}

int Response::handlePost(requestInfo &request, Location &location)
{
	if (!location.upload_dir.empty())
	{
		std::string uploadPath = location.upload_dir;
		if (uploadPath[uploadPath.size() - 1] != '/')
			uploadPath += '/';

		std::string filename = request.filename;
		if (filename.empty())
			filename = "file";

		std::string fullPath = uploadPath + filename;

		if (!fileExists(uploadPath) || !isDirectory(uploadPath))
			request.status = 404;
		else
		{
			std::ofstream file(fullPath.c_str(), std::ios::binary);
			if (file.is_open())
			{
				file.write(request.body.c_str(), request.body.size());
				file.close();
				request.status = 201;
				return (1);
			}
			else
				request.status = 500;
		}
	}
	else
		request.status = 403;
	return (0);
}

void Response::handleDelete(serverInfo &server, requestInfo &request)
{
	if (Response::fileExists(server.link))
	{
		if (unlink(server.link.c_str()) == 0)
			request.status = 204;
		else
			request.status = 403;
	}
	else
		request.status = 404;
}

std::string Response::getCompleteResponse(void)
{
	return (_header + _body);
}

void Response::generateHeader(int statusCode, std::string contentType, std::string location)
{
	std::ostringstream header;
	header << "HTTP/1.1 " << statusCode << " " << getStatusMessage(statusCode) << "\r\n";
	header << "Content-Length: " << _body.size() << "\r\n";

	if (!contentType.empty())
		header << "Content-Type: " << contentType << "\r\n";

	if (!location.empty())
		header << "Location: " << location << "\r\n";

	header << "Connection: close\r\n";
	header << "\r\n";

	_header = header.str();
}

int Response::generateAutoindex(std::string path, std::string uri)
{
	DIR *dir = opendir(path.c_str());
	if (!dir)
		return (500);

	std::ostringstream html;
	html << "<!DOCTYPE html>\n<html>\n<head>\n<title>" << uri << "</title>\n</head>\n<body>\n";
	html << "<h1>Directory listing for " << uri << "</h1>\n<ul>\n";

	if (uri != "/")
	{
		html << "<li><a href=\"../\">../</a></li>\n";
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;
		if (name == "." || name == "..")
		{
			continue;
		}

		std::string fullPath = path + "/" + name;
		if (isDirectory(fullPath))
		{
			html << "<li><a href=\"" << name << "/\">" << name << "/</a></li>\n";
		}
		else
		{
			html << "<li><a href=\"" << name << "\">" << name << "</a></li>\n";
		}
	}

	html << "</ul>\n</body>\n</html>";
	closedir(dir);

	_body = html.str();
	return (200);
}

void Response::defaultHtmlBody(int statusCode)
{
	std::string message = getStatusMessage(statusCode);
	std::ostringstream body;
	std::ostringstream header;

	body << "<html>\n"
	     << "<head><title>" << statusCode << " " << message << "</title></head>\n"
	     << "<body>\n"
	     << "<center><h1>" << statusCode << " " << message << "</h1></center>\n"
	     << "<hr>\n"
	     << "</body>\n"
	     << "</html>\n";

	_body = body.str();
	generateHeader(statusCode, "text/html", "");
}
void Response::setBody(std::string body)
{
	_body = body;
}

std::string Response::getMimeType(std::string filename)
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

std::string Response::getStatusMessage(int statusCode)
{
	switch (statusCode)
	{
		case 100:
			return ("Continue");
		case 101:
			return ("Switching Protocols");
		case 102:
			return ("Processing");
		case 103:
			return ("Early Hints");

		case 200:
			return ("OK");
		case 201:
			return ("Created");
		case 202:
			return ("Accepted");
		case 203:
			return ("Non-Authoritative Information");
		case 204:
			return ("No Content");
		case 205:
			return ("Reset Content");
		case 206:
			return ("Partial Content");
		case 207:
			return ("Multi-Status");
		case 208:
			return ("Already Reported");
		case 226:
			return ("IM Used");

		case 300:
			return ("Multiple Choices");
		case 301:
			return ("Moved Permanently");
		case 302:
			return ("Found");
		case 303:
			return ("See Other");
		case 304:
			return ("Not Modified");
		case 305:
			return ("Use Proxy");
		case 307:
			return ("Temporary Redirect");
		case 308:
			return ("Permanent Redirect");

		case 400:
			return ("Bad Request");
		case 401:
			return ("Unauthorized");
		case 402:
			return ("Payment Required");
		case 403:
			return ("Forbidden");
		case 404:
			return ("Not Found");
		case 405:
			return ("Method Not Allowed");
		case 406:
			return ("Not Acceptable");
		case 407:
			return ("Proxy Authentication Required");
		case 408:
			return ("Request Timeout");
		case 409:
			return ("Conflict");
		case 410:
			return ("Gone");
		case 411:
			return ("Length Required");
		case 412:
			return ("Precondition Failed");
		case 413:
			return ("Payload Too Large");
		case 414:
			return ("URI Too Long");
		case 415:
			return ("Unsupported Media Type");
		case 416:
			return ("Range Not Satisfiable");
		case 417:
			return ("Expectation Failed");
		case 418:
			return ("I'm a teapot");
		case 421:
			return ("Misdirected Request");
		case 422:
			return ("Unprocessable Content");
		case 423:
			return ("Locked");
		case 424:
			return ("Failed Dependency");
		case 425:
			return ("Too Early");
		case 426:
			return ("Upgrade Required");
		case 428:
			return ("Precondition Required");
		case 429:
			return ("Too Many Requests");
		case 431:
			return ("Request Header Fields Too Large");
		case 451:
			return ("Unavailable For Legal Reasons");

		case 500:
			return ("Internal Server Error");
		case 501:
			return ("Not Implemented");
		case 502:
			return ("Bad Gateway");
		case 503:
			return ("Service Unavailable");
		case 504:
			return ("Gateway Timeout");
		case 505:
			return ("HTTP Version Not Supported");
		case 506:
			return ("Variant Also Negotiates");
		case 507:
			return ("Insufficient Storage");
		case 508:
			return ("Loop Detected");
		case 510:
			return ("Not Extended");
		case 511:
			return ("Network Authentication Required");

		default:
			return ("Unknown");
	}
}

bool Response::fileExists(std::string path)
{
	struct stat st;
	return (stat(path.c_str(), &st) == 0);
}

bool Response::isDirectory(std::string path)
{
	struct stat st;
	if (stat(path.c_str(), &st) != 0)
	{
		return (false);
	}
	return (S_ISDIR(st.st_mode));
}
