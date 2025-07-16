#include <stdio.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include "Epoll.hpp"
#include "Socket.hpp"

#define PORT "8080"
#define HOST "127.0.0.1"

int main(int argc, char const *argv[])
{
    try{
        Socket  newSocket(HOST, PORT);
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    Socket  newSocket(HOST, PORT);
    int     socket_fd;
    Epoll   epoll;
    struct epoll_event ev;
    struct epoll_event* events;
    int     nr_events;
    int     new_socket;

    std::string hello = "hello from server";
    socket_fd = newSocket.getFd();
    ev.events =  EPOLLIN | EPOLLOUT;
    ev.data.fd = socket_fd;
    epoll_ctl(epoll.getEpfd(), EPOLL_CTL_ADD, socket_fd, &ev);
    while(1)
    {
        nr_events = epoll.wait();
        events = epoll.getEvents();
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = newSocket.accept())<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }
        if (new_socket != socket_fd)
        {
            epoll.add_fd(new_socket);
            char buffer[3000] = {0};
            for (int i = 0; i < nr_events; i++)
		    {
			    int valread = read(events[i].data.fd, buffer, 3000);
        	    printf("\n%s\n",buffer );
        	    memset(buffer, 0,  3000);
                if (valread > 0)
			        write(events[i].data.fd, hello.c_str() , hello.size());
        	    printf("------------------Hello message sent-------------------\n");
		    }
        }
        //epoll.remove_fd(new_socket);
        // close(new_socket);
    }
    epoll.remove_fd(socket_fd);
    return 0;
}
