#pragma once

#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

class Epoll
{
   public:
	Epoll(void);
	~Epoll(void);

	int addFd(int fd);
	int removeFd(int fd);
	int modifyFd(int fd, int events);
	int wait(void);
	struct epoll_event* getEvents(void);

   private:
	int _epollFd;
	struct epoll_event _events[1024];
};
