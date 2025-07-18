#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include "Epoll.hpp"
#include "Socket.hpp"
#include "Request.hpp"

#define PORT "8080"
#define HOST "127.0.0.1"

int main(int argc, char const* argv[])
{
	std::vector<Server>	main_vector;

	if (argc != 2)
	{
		std::cerr << "error1\n";
		return (1);
	}
	 if (std::strlen(argv[1]) < 5 || std::strcmp(argv[1] + std::strlen(argv[1]) - 5, ".conf"))
	 {
       std::cerr << "error2\n";
		return (1);
    }
	std::ifstream file(argv[1]);
    if (!file)
	{
        std::cerr << "error3\n";
    }
	try
	{
		readFileAsString(file, main_vector);
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
		file.close();
		return (1);
	}
	file.close();
	try
	{
		Socket newSocket(HOST, PORT);
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
	Socket newSocket(HOST, PORT);
	int socket_fd;
	Epoll epoll;
	struct epoll_event ev;
	struct epoll_event* events;
	int nr_events;
	int new_socket;

	std::string hello =
	    "HTTP/1.1 200 OK\r\n"
	    "Content-Type: text/plain\r\n"
	    "Content-Length: 15\r\n"
	    "Connection: close\r\n"
	    "\r\n"
	    "Hello, browser!";
	socket_fd = newSocket.getFd();
	ev.events = EPOLLIN | EPOLLOUT;
	ev.data.fd = socket_fd;
	epoll_ctl(epoll.getEpfd(), EPOLL_CTL_ADD, socket_fd, &ev);
	char buffer[3000] = {0};
	while (1)
	{
		nr_events = epoll.wait();
		events = epoll.getEvents();
		for (int i = 0; i < nr_events; i++)
		{
			if (events[i].data.fd == socket_fd)
			{
				new_socket = newSocket.accept();
				if (new_socket < 0)
					break;
				epoll.add_fd(new_socket);
			}
			else
			{
				int valread = read(events[i].data.fd, buffer, 3000);
				if (valread <= 0)
					continue;
				else
				{
					printf("\n%s\n", buffer);
					Request request(buffer);
					request.checkServer(main_vector);
					request.printInfoRequest();
					memset(buffer, 0, 3000);
					write(events[i].data.fd, hello.c_str(), hello.size());
				}
				printf("------------------Hello message sent-------------------\n");
			}
		}
	}
	epoll.remove_fd(socket_fd);
	newSocket.close();
	return (0);
}
