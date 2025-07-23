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

	int add_fd(int fd);
	int remove_fd(int fd);
	int wait(void);
	int getEpfd(void) const;
	struct epoll_event* getEvents(void);

   private:
	int epfd;
	struct epoll_event events[1024];
};
