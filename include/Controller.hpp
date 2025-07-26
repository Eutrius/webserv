#pragma once

#include <sys/socket.h>
#include <unistd.h>
#include <map>
#include <string>
#include <vector>
#include "parser.hpp"

#define BUFFER_SIZE 8192

struct Connection
{
	std::string request;
	std::string response;
	size_t sent;
	time_t lastActivity;
	std::vector<Server> servers;
};

class Controller
{
   public:
	Controller(void);
	~Controller(void);

	Connection &getConnection(int fd);

	void newConnection(int fd, std::vector<Server> servers);
	void closeConnection(int fd);
	int read(int fd);
	int write(int fd);

   private:
	std::map<int, Connection> _connections;
};
