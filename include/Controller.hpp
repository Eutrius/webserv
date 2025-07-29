#pragma once

#include <sys/socket.h>
#include <unistd.h>
#include <map>
#include <string>
#include <vector>
#include "Epoll.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Socket.hpp"
#include "parser.hpp"

#define BUFFER_SIZE 8192

enum con_type
{
	CON_SERVER = 1,
	CON_CLIENT = 1 << 1,
	CON_CGI = 1 << 2,
};

struct Connection
{
	con_type type;
	std::string readBuffer;
	std::string writeBuffer;
	size_t sent;
	time_t lastActivity;
	std::vector<Server> servers;
	Socket socket;
	int targetFd;
	bool isWaiting;
	Request req;
	Response res;
};

class Controller
{
   public:
	Controller(Epoll &epoll);
	~Controller(void);

	int initServers(std::vector<Socket> &sockets);
	void newClientConnection(int fd);
	void newCGIConnection(int fd, int targetFd);
	void newServerConnection(Socket socket);
	void closeConnection(int fd);
	int handleRequest(int fd);
	int read(int fd);
	int write(int fd);

	Connection &getConnection(int fd);
	con_type getConnectionTypeByFd(int fd);

   private:
	std::map<int, Connection> _connections;
	Epoll &_epoll;
};
