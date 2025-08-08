#pragma once

#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <ctime>
#include <map>
#include <string>
#include <vector>
#include "Epoll.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Socket.hpp"
#include "parser.hpp"

#define BUFFER_SIZE 42
#define TIMEOUT 15
#define CGI_TIMEOUT 30

enum con_type
{
	CON_SERVER = 1,
	CON_CLIENT = 1 << 1,
	CON_CGI = 1 << 2,
};

struct Connection
{
	Request req;
	Response res;
	Socket socket;
	con_type type;

	std::string readBuffer;
	std::string writeBuffer;
	size_t sent;
	int targetFd;
	int pid;
	time_t lastActivity;
};

class Controller
{
   public:
	Controller(Epoll &epoll);
	~Controller(void);

	Connection &getConnection(int fd);
	con_type getConnectionTypeByFd(int fd);
	Response &getResponseByFd(int fd);
	Request &getRequestByFd(int fd);

	bool isValidConnection(int fd);

	int initServers(std::vector<Socket> &sockets);
	void newClientConnection(int fd);
	void newCGIConnection(int fd, int targetFd, int event);
	void newServerConnection(Socket socket);
	void closeConnection(int fd);
	void modifyConnection(int fd, int event);
	void checkTimeouts(void);

	int handleRequest(int fd, std::vector<std::string> cookie);
	int handleCGI(int fd);
	void handleCGIOutput(int fd);
	int read(int fd);
	int write(int fd);

	int initPipes(int inPipe[2], int outPipe[2]);
	std::string extractAdditionalHeaders(std::string header);
	void generateCGIEnv(std::vector<char *> &envp, std::vector<std::string> &envStrings, serverInfo &server,
	                    requestInfo &request, t_host host);
	std::string normalizeEnvName(std::string headerName);
	std::string itoaIP(int ip);

   private:
	std::map<int, Connection> _connections;
	std::map<int, time_t> _cgiConnections;
	Epoll &_epoll;
};
