#include "Epoll.hpp"

Epoll::Epoll(void)
{
	_epollFd = epoll_create(1);
	if (_epollFd == -1)
		throw std::runtime_error("Epoll: failed to create epoll");
}

Epoll::~Epoll(void)
{
	close(_epollFd);
}

int Epoll::addFd(int fd)
{
	int nr_fd;
	struct epoll_event ev;

	ev.events = EPOLLIN;
	ev.data.fd = fd;
	nr_fd = epoll_ctl(this->_epollFd, EPOLL_CTL_ADD, fd, &ev);
	if (nr_fd == -1)
		throw std::runtime_error("Epoll: failed to add fd to epoll");
	return (0);
}

int Epoll::removeFd(int fd)
{
	int nr_fd;

	nr_fd = epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, fd, NULL);
	return (nr_fd);
}

int Epoll::modifyFd(int fd, int events)
{
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;

	if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &ev) < 0)
	{
		throw std::runtime_error("Failed to modify fd in epoll");
	}
	return (0);
}

int Epoll::wait(void)
{
	int check;

	check = epoll_wait(this->_epollFd, _events, 1024, -1);
	if (check == -1)
		std::cout << "Problems with wait" << std::endl;
	return (check);
}

struct epoll_event* Epoll::getEvents(void)
{
	return (_events);
}
