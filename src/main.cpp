#include <stdio.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include "Epoll.hpp"

#define PORT 8080

int main(int argc, char const *argv[])
{
    int server_fd, new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    Epoll   object;
    struct epoll_event ev;
    int max_events;
    struct epoll_event* events;
    int i = 0;

    char *hello = "Hello from server";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }


    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    memset(address.sin_zero, '\0', sizeof address.sin_zero);


    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    epoll_ctl(object.getEpfd(), EPOLL_CTL_ADD, server_fd, &ev);
    while(1)
    {
        max_events = object.wait();
        events = object.getEvents();
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }
        if (new_socket != server_fd)
        {
            object.add_fd(new_socket);
            char buffer[3000] = {0};
            for (i = 0; i < max_events; i++)
		    {
			    valread = read(events[i].data.fd, buffer, 3000);
        	    printf("\n%s\n",buffer );
        	    memset(buffer, 0,  3000);
                if (valread > 0)
			        write(events[i].data.fd, hello , strlen(hello));
        	    printf("------------------Hello message sent-------------------\n");
		    }
        }
        //object.remove_fd(new_socket);
        // close(new_socket);
    }
    object.remove_fd(server_fd);
    return 0;
}
