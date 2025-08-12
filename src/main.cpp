#include <csignal>
#include <cstdlib>
#include "Controller.hpp"
#include "Epoll.hpp"
#include "Request.hpp"
#include "Socket.hpp"

int serverState = 0;
static bool parseConfig(int argc, char const *argv[], t_serversMap &serversMap);
static void handleSignal(int signal);

int main(int argc, char const *argv[])
{
	std::srand(std::time(NULL));
	t_serversMap serversMap;
	Cookie cookie;
	if (!parseConfig(argc, argv, serversMap))
		return (1);

	std::vector<Socket> sockets = Socket::initSockets(serversMap);

	Epoll epoll;
	if (epoll.getFd() == -1)
		return (1);

	Controller controller(epoll);
	if (controller.initServers(sockets))
	{
		return (1);
		std::cerr << "Webserv: no server created." << std::endl;
	}

	signal(SIGINT, handleSignal);
	serverState = 1;
	std::cout << "Webserv: server started successfully." << std::endl;
	while (serverState)
	{
		int nEvents = epoll.wait();
		struct epoll_event *events = epoll.getEvents();

		controller.checkTimeouts();
		for (int i = 0; i < nEvents; i++)
		{
			int fd = events[i].data.fd;
			uint32_t eventFlags = events[i].events;

			if (eventFlags & EPOLLERR || !controller.isValidConnection(fd))
			{
				controller.closeConnection(fd);
				continue;
			}

			Connection &curr = controller.getConnection(fd);
			con_type type = curr.type;

			if (eventFlags & (EPOLLIN | EPOLLHUP))
			{
				if (type & CON_SERVER)
				{
					controller.newClientConnection(fd);
					continue;
				}

				if (eventFlags & EPOLLHUP && type & CON_CLIENT)
				{
					controller.closeConnection(fd);
					continue;
				}

				int bytesRead = controller.read(fd);
				if (bytesRead == -1)
					controller.closeConnection(fd);
				else if (type & CON_CLIENT)
				{
					if (checkBody(curr.readBuffer))
					{
						Request req(curr.readBuffer, curr.socket.getServers());
						if (req.getInfo().newClient == true)
							cookie.createCookie();
						else
							cookie.analizeCookie(req.getInfo().cookie);

						if (controller.handleRequest(fd, cookie.getClients()[cookie.getCurrentClient()].info))
							controller.modifyConnection(fd, EPOLLOUT);
					}
				}
				else if (type & CON_CGI && bytesRead == 0)
					controller.handleCGIOutput(fd);
			}
			else if (eventFlags & EPOLLOUT)
			{
				int bytesSent = controller.write(fd);
				if (bytesSent == -1 || (type & CON_CGI && bytesSent == 0))
					controller.closeConnection(fd);
				else if (type & CON_CLIENT)
				{
					if (curr.sent >= curr.writeBuffer.length())
						controller.closeConnection(fd);
				}
			}
		}
	}
	std::cout << "Websev: server shutdown complete." << std::endl;
	return (0);
}

static bool parseConfig(int argc, char const *argv[], t_serversMap &serversMap)
{
	if (argc != 2)
	{
		std::cerr << "The program requires a server configuration file as an argument\n";
		return (false);
	}
	if (std::strlen(argv[1]) < 5 || std::strcmp(argv[1] + std::strlen(argv[1]) - 5, ".conf"))
	{
		std::cerr << "Configuration file must have a .conf extension\n";
		return (false);
	}
	std::ifstream file(argv[1]);
	if (!file)
	{
		std::cerr << "Failed to read configuration file\n";
		return (false);
	}
	std::vector<Server> servers;
	try
	{
		readFileAsString(file, servers);
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
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
	std::cout << "\n";
	serverState = 0;
}
