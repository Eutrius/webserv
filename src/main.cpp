#include "Epoll.hpp"
#include "Request.hpp"
#include "Socket.hpp"
#include <netinet/in.h>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT "8080"
#define HOST "127.0.0.1"

static bool parseConfig(int argc, char const *argv[], std::vector<Server> &Servers)
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
        readFileAsString(file, Servers);
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
    std::vector<Socket *> sockets;

    Epoll epoll;
    std::set<std::pair<int, int> > uniqueListen;

    if (!parseConfig(argc, argv, servers))
        return (1);

    for (size_t i = 0; i < servers.size(); i++)
        uniqueListen.insert(servers[i].listen.begin(), servers[i].listen.end());

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

    struct epoll_event *events;
    int nr_events;
    int new_socket;

    std::string hello = "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: 15\r\n"
                        "Connection: close\r\n"
                        "\r\n"
                        "Hello, browser!";

    for (size_t i = 0; i < sockets.size(); i++)
    {
        int fd = sockets[i]->getFd();
        epoll.add_fd(fd);
    }
    char buffer[3000] = {0};
    while (1)
    {
        nr_events = epoll.wait();
        events = epoll.getEvents();
        for (int i = 0; i < nr_events; i++)
        {
            for (size_t j = 0; j < sockets.size(); j++)
            {
                if (events[i].data.fd == sockets[j]->getFd())
                {
                    new_socket = sockets[j]->accept();
                    if (new_socket < 0)
                        break;
                    epoll.add_fd(new_socket);
                }
            }
            int valread = read(events[i].data.fd, buffer, 3000);
            if (valread <= 0)
                continue;
            else
            {
                printf("\n%s\n", buffer);
                Request request(buffer);
                request.checkServer(servers);
                request.printInfoRequest();
                memset(buffer, 0, 3000);
                write(events[i].data.fd, hello.c_str(), hello.size());
            }
            printf("------------------Hello message sent-------------------\n");
        }
    }
    return (0);
}
