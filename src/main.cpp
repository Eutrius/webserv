#include <csignal>
#include "Controller.hpp"
#include "Epoll.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Socket.hpp"

int serverState;
static bool parseConfig(int argc, char const *argv[], t_serversMap &serversMap);
static void handleSignal(int signal);

int main(int argc, char const *argv[])
{
	t_serversMap serversMap;
	if (!parseConfig(argc, argv, serversMap))
		return (1);

	std::vector<Socket> sockets = Socket::initSockets(serversMap);

	Epoll epoll;
	epoll.addFds(sockets);

	if (sockets.empty())
	{
		std::cerr << "Webserv: no sockets created" << std::endl;
		return (1);
	}

	Controller controller;
	signal(SIGINT, handleSignal);
	serverState = 1;
	std::cout << "Server started" << std::endl;
	while (serverState)
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
				controller.closeConnection(fd);
				continue;
			}

			bool isServerFd = false;
			size_t j;
			for (j = 0; j < sockets.size(); j++)
			{
				if (fd == sockets[j].getFd())
				{
					isServerFd = true;
					break;
				}
			}

			if (isServerFd && (eventFlags & EPOLLIN))
			{
				int newFd = sockets[j].accept();
				if (newFd > 0)
				{
					try
					{
						epoll.addFd(newFd);
						controller.newConnection(newFd, sockets[j].getServers());
					}
					catch (std::exception &e)
					{
						std::cout << e.what() << std::endl;
						close(fd);
					}
				}
			}
			else if (eventFlags & EPOLLIN)
			{
				if (controller.read(fd))
				{
					epoll.removeFd(fd);
					controller.closeConnection(fd);
				}

				Connection &curr = controller.getConnection(fd);
				if (checkBody(curr.request))
				{
					Request req(curr.request);
					req.checkServer(curr.servers);
					std::cout << curr.request << std::endl;
					req.printInfoRequest();
					Response res;
					curr.response = res.getCompleteResponse();
					epoll.modifyFd(fd, EPOLLOUT);
				}
			}
			else if (eventFlags & EPOLLOUT)
			{
				if (controller.write(fd))
				{
					epoll.removeFd(fd);
					controller.closeConnection(fd);
				}
			}
		}
	}
	Socket::closeSockets(sockets);
	std::cout << "Server shutdown" << std::endl;
	return (0);
}

static bool parseConfig(int argc, char const *argv[], t_serversMap &serversMap)
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
	std::vector<Server> servers;
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

	for (size_t i = 0; i < servers.size(); i++)
	{
		std::vector<t_host> listen = servers[i].listen;
		for (size_t j = 0; j < listen.size(); j++)
			serversMap[listen[j]].push_back(servers[i]);
	}

	file.close();
	return (true);
}

static void handleSignal(int signal)
{
	(void) signal;
	serverState = 0;
}
