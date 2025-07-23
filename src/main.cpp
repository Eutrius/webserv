#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <set>
#include "Epoll.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Socket.hpp"
#include "map"
#include "parser.hpp"

#define BUFFER_SIZE 8192

static bool parseConfig(int argc, char const *argv[], std::vector<Server> &servers)
{
	if (argc != 2)
	{
		std::cerr << "error1\n";
		return (false);
	}
	if (std::strlen(argv[1]) < 5 || std::strcmp(argv[1] + std::strlen(argv[1]) - 5, ".conf"))
	{
		std::cerr << "error2\n";
		return (false);
	}
	std::ifstream file(argv[1]);
	if (!file)
	{
		std::cerr << "error3\n";
		return (false);
	}
	try
	{
		readFileAsString(file, servers);
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
		file.close();
		return (false);
	}
	file.close();
	return (true);
}

int main(int argc, char const *argv[])
{
	std::vector<Server> servers;
	if (!parseConfig(argc, argv, servers))
		return (1);

	std::set<std::pair<int, int> > uniqueListen;
	for (size_t i = 0; i < servers.size(); i++)
		uniqueListen.insert(servers[i].listen.begin(), servers[i].listen.end());

	std::vector<Socket *> sockets;
	try
	{
		std::set<std::pair<int, int> >::iterator it;
		for (it = uniqueListen.begin(); it != uniqueListen.end(); ++it)
		{
			std::pair<int, int> listen = *it;
			sockets.push_back(new Socket(listen.first, listen.second));
		}
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}

	Epoll epoll;
	for (size_t i = 0; i < sockets.size(); i++)
	{
		int fd = sockets[i]->getFd();
		epoll.addFd(fd);
	}

	std::map<int, std::string> requests;
	std::map<int, std::string> responses;
	std::map<int, size_t> sent;
	std::map<int, time_t> lastActivity;
	char buffer[BUFFER_SIZE];
	(void) buffer;
	while (1)
	{
		int nEvents = epoll.wait();
		struct epoll_event *events = epoll.getEvents();
		for (int i = 0; i < nEvents; i++)
		{
			int fd = events[i].data.fd;
			uint32_t eventFlags = events[i].events;
			if (eventFlags & (EPOLLHUP | EPOLLERR))
			{
				epoll.removeFd(fd);
				requests.erase(fd);
				responses.erase(fd);
				close(fd);
				continue;
			}

			bool isServerFd = false;
			size_t j;
			for (j = 0; j < sockets.size(); j++)
			{
				if (events[i].data.fd == sockets[j]->getFd())
				{
					isServerFd = true;
					break;
				}
			}

			if (isServerFd && (eventFlags & EPOLLIN))
			{
				int clientFd = sockets[j]->accept();
				if (clientFd > 0)
					epoll.addFd(clientFd);
			}
			else if (eventFlags & EPOLLIN)
			{
				char buffer[BUFFER_SIZE];
				int bytes_read = recv(fd, buffer, sizeof(buffer) - 1, 0);
				std::cout << buffer << std::endl;
				if (bytes_read > 0)
				{
					requests[fd].append(buffer, bytes_read);
					lastActivity[fd] = time(NULL);
				}
				else
				{
					epoll.removeFd(fd);
					requests.erase(fd);
					responses.erase(fd);
					close(fd);
				}
				if (checkBody(requests[fd]))
				{
					Request req(requests[fd]);
					std::cout << requests[fd] << std::endl;
					requests.erase(fd);
					Response res;
					responses[fd] = res.getCompleteResponse();
					epoll.modifyFd(fd, EPOLLOUT);
				}
			}
			else if (eventFlags & EPOLLOUT)
			{
				int bytes_sent = send(fd, responses[fd].c_str() + sent[fd], responses[fd].length() - sent[fd], 0);
				if (bytes_sent > 0)
				{
					sent[fd] += bytes_sent;
					lastActivity[fd] = time(NULL);
				}

				if (bytes_sent <= 0 || sent[fd] > responses[fd].length())
				{
					epoll.removeFd(fd);
					responses.erase(fd);
					sent.erase(fd);
					close(fd);
				}
			}
		}
	}
	return (0);
}
