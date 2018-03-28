#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/types.h>
#include "../include/client_event.h"

#define SERV_PORT 8080
#define MAX_EVENTS 10

void handle_event(const int fd)
{
	meg client_info;

	int readn;
	readn = read(fd, &client_info, sizeof(client_info));
	if (readn > 0)
	{
		fprintf(stdout, "the content from socket: %d\n", client_info.my_pid);
	}
}

static int setnonblocking(const int sockfd)
{
	int ret = fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0)|O_NONBLOCK);
	if (ret < 0)
	{
		return -1;
	}

	return 0;
}

static int init_lfd(void)
{
	int sockfd;
	int ret;
	struct sockaddr_in serv_addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0)
	{
		perror("24N--Create socket file failed:");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(SERV_PORT);

	int opt = 1;
	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));
	if (ret < 0)
	{
		perror("49N--Setsockopt failed:");
		return -1;
	}

	ret = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (ret < 0)
	{
		perror("38N--Bind attribute failed:");
		return -1;
	}

	return sockfd;
}

static void pack_epoll(const int sockfd)
{
	int cfd, i;
	int error_connect = 0;
	struct sockaddr_in clie_addr;
	socklen_t clie_addr_len;
	char client_addr[INET_ADDRSTRLEN];

	int epollfd, nfds;
	struct epoll_event ev, events[MAX_EVENTS];
	int ret;

	//epoll: create red-black tree
	epollfd = epoll_create(10);

	//make lfd is listened by epoll
	ev.data.fd = sockfd;
	ev.events = EPOLLIN;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev) < 0)
	{
		perror("epoll_ctl add sockfd");
		return;
	}

	//to check witch fd has event;
	for ( ; ;)
	{
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		int n;
		if (nfds == -1)
		{
			perror("epoll_pwait");
			return;
		}

		for (n = 0; n < nfds; ++n)
		{
			if (events[n].data.fd == sockfd)
			{
				cfd = accept(sockfd, (struct sockaddr *)&clie_addr, &clie_addr_len);
				if (cfd == -1)
				{
					perror("accept failed");
					return;
				}

				ret = setnonblocking(cfd);
				if (ret < 0)
				{
					error_connect++;
					if(error_connect >= 5)
					{
						return ;
					}
					fprintf(stderr, "can set %d fd nonblocking", cfd);
					continue;
				}
				ev.data.fd = cfd;
				ev.events = EPOLLIN | EPOLLET;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, cfd, &ev) < 0)
				{
					error_connect++;
					if(error_connect >= 5)
					{
						return ;
					}
					fprintf(stderr, "can add %d fd into listen list", cfd);
					continue;
				}
				fprintf(stdout, "recive client from %s:%d\n", inet_ntop(AF_INET, &clie_addr.sin_addr.s_addr, client_addr, INET_ADDRSTRLEN), ntohs(clie_addr.sin_port));
			}else
			{
				handle_event(events[n].data.fd);
			}

		}

	}
}

void main(void)
{
	int lfd, ret;

	//init listen socket file
	lfd = init_lfd();
	if (lfd < 0)
	{
		fprintf(stdout, "listen socket init failed");
		exit(1);
	}

	if(listen(lfd, 64) < 0)
	{
		perror("62N--Listen:");
		exit(1);
	}

	pack_epoll(lfd);

	return ;
}
