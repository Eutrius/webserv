#pragma once

#include <sys/socket.h>
#include <unistd.h>
#include <map>
#include <string>
#include <vector>
#include "Epoll.hpp"
#include "parser.hpp"

#define BUFFER_SIZE 8192

enum con_type
{
	CON_SERVER = 1,
	CON_CLIENT = 1 << 1,
	CON_CGI = 1 << 2,
	CON_FILE = 1 << 3,
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
};

class Controller
{
   public:
	Controller(void);
	~Controller(void);

	void newClientConnection(Epoll &epoll, int fd);
	void newServerConnection(Socket socket);
	void closeConnection(int fd);
	int read(int fd);
	int write(int fd);

	Connection &getConnection(int fd);
	con_type getConnectionTypeByFd(int fd);

   private:
	std::map<int, Connection> _connections;
};
