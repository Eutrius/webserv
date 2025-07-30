#include <csignal>
#include "Controller.hpp"
#include "Epoll.hpp"
#include "Request.hpp"
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
	Controller controller;
	Epoll epoll;
	if (epoll.getFd() == -1)
		return (1);

	std::vector<Socket>::iterator it = sockets.begin();
	while (it != sockets.end())
	{
		try
		{
			epoll.addFd(it->getFd());
			controller.newServerConnection(*it);
			it++;
		}
		catch (std::exception &e)
		{
			std::cout << e.what() << std::endl;
			it = sockets.erase(it);
		}
	}

	if (sockets.empty())
	{
		std::cerr << "Webserv: no sockets created" << std::endl;
		return (1);
	}

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
			}
			else if (eventFlags & EPOLLIN)
			{
				con_type type = controller.getConnectionTypeByFd(fd);

				if (type & CON_SERVER)
					controller.newClientConnection(epoll, fd);
				else
				{
					int bytes_read = controller.read(fd);
					if (bytes_read == -1)
						continue;

					if (type & CON_CLIENT)
					{
						Connection &curr = controller.getConnection(fd);
						if (checkBody(curr.readBuffer))
						{
							Request req(curr.readBuffer);
							try
							{
								req.checkServer(curr.servers);
							}
							catch(const std::exception& e)
							{
								std::cerr << e.what() << '\n';
								continue ;
							}
							if (controller.handleRequest(fd))
								epoll.modifyFd(fd, EPOLLOUT);
							else
								epoll.modifyFd(fd, 0);
						}
					}
					else if (bytes_read < BUFFER_SIZE && type & (CON_CGI | CON_FILE))
					{
						Connection &curr = controller.getConnection(fd);
						Connection &target = controller.getConnection(curr.targetFd);
						(void) target;
					}
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
