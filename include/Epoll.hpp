#pragma once

#include <fcntl.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include "Socket.hpp"

class Epoll
{
   public:
	Epoll(void);
	~Epoll(void);

	int getFd(void) const;
	struct epoll_event* getEvents(void);

	void addFd(int fd);
	int removeFd(int fd);
	int modifyFd(int fd, int events);
	int wait(void);

   private:
	int _epollFd;
	struct epoll_event _events[1024];
};
