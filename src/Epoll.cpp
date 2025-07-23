#include "Epoll.hpp"

Epoll::Epoll(void)
{
	epfd = epoll_create(1);
	if (epfd == -1)
		std::cout << "It was not possible to create the epoll" << std::endl;
	else
		std::cout << "Epoll created" << std::endl;
}

Epoll::~Epoll(void)
{
	close(epfd);
}

int Epoll::add_fd(int fd)
{
	int nr_fd;
	struct epoll_event ev;

	ev.events = EPOLLIN | EPOLLOUT;
	ev.data.fd = fd;
	nr_fd = epoll_ctl(this->epfd, EPOLL_CTL_ADD, fd, &ev);
	if (nr_fd == -1)
		std::cout << "Failed to add fd to epoll" << std::endl;
	else
		std::cout << "Fd added to epoll" << std::endl;
	return (nr_fd);
}

int Epoll::remove_fd(int fd)
{
	int nr_fd;

	nr_fd = epoll_ctl(this->epfd, EPOLL_CTL_DEL, fd, NULL);
	return (nr_fd);
}

int Epoll::wait(void)
{
	int check;

	check = epoll_wait(this->epfd, events, 1024, -1);
	if (check == -1)
		std::cout << "Problems with wait" << std::endl;
	return (check);
}

int Epoll::getEpfd(void) const
{
	return (this->epfd);
}

struct epoll_event* Epoll::getEvents(void)
{
	return (events);
}
