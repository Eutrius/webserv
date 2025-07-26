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

void Epoll::addFd(int fd)
{
	int nr_fd;
	struct epoll_event ev;

	ev.events = EPOLLIN;
	ev.data.fd = fd;
	nr_fd = epoll_ctl(this->_epollFd, EPOLL_CTL_ADD, fd, &ev);
	if (nr_fd == -1)
		throw std::runtime_error("Epoll: failed to add fd to epoll");
	std::cout << "Epoll added" << std::endl;
}

void Epoll::addFds(std::vector<Socket>& sockets)
{
	std::vector<Socket>::iterator it = sockets.begin();
	while (it != sockets.end())
	{
		try
		{
			addFd(it->getFd());
			it++;
		}
		catch (std::exception& e)
		{
			std::cout << e.what() << std::endl;
			it = sockets.erase(it);
		}
	}
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
		throw std::runtime_error("Epoll: failed to modify fd event in epoll");
	}
	return (0);
}

int Epoll::wait(void)
{
	int nEvents;

	nEvents = epoll_wait(this->_epollFd, _events, 1024, -1);
	if (nEvents == -1)
	{
		if (errno == EINTR)
			return (nEvents);
		std::cerr << "Epoll: error during epoll wait" << std::endl;
	}
	return (nEvents);
}

struct epoll_event* Epoll::getEvents(void)
{
	return (_events);
}
