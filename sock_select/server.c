#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>

#define MAX_CLIENT 128
#define SERV_PORT 6666

#ifdef BUFSIZ
#undef BUFSIZ
#define BUFSIZ 1024
#endif
#define KEY "2015032042"

typedef struct{
	int fd;
	short port;
	char ip[INET_ADDRSTRLEN];
	int index;
	char original[BUFSIZ];
	int cindex;
	char ciphertext[BUFSIZ];
}client_info;

static void 
decryption(const char* source, char* dest, int len)
{
	//int len = strlen(source);
	int offset;
	int i;
	for(offset = 0; offset < len; offset += 10)
	{
		for (i = 0; i < 10; i++)
		{
			dest[offset + i] = (source[offset + i] - KEY[i]);
		}
	}
}

static void 
save_history(const char *message, char *history, int index, int len)
{
	memcpy(history+index, message, len);
}

void 
perr_exit(char *errmsg)
{
	perror(errmsg);
	exit(EXIT_FAILURE);
}

int
main(void)
{
	fd_set rfds, orgset;
	client_info client[MAX_CLIENT];
	int listenfd, connfd, maxfd, sockfd;
	int i, j, maxi, nready, retval;
	char client_ip[INET_ADDRSTRLEN], msgbuf[BUFSIZ], message[BUFSIZ];

	struct sockaddr_in serv_addr, clie_addr;
	socklen_t clie_attr_len = sizeof(clie_addr);

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0)
		perr_exit("socket create");

	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int opt = 1;
	retval = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));
	if (retval < 0)
		perr_exit("setsockopt");

	retval = bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (retval < 0)
		perr_exit("bind");

	retval = listen(listenfd, 128);
	if (retval < 0)
		perr_exit("liste");

	for (i = 0; i < MAX_CLIENT; i++)
		client[i].fd = -1;

	FD_ZERO(&orgset);
	FD_SET(listenfd, &orgset);

	maxfd = listenfd;
	maxi = -1;


	while (1)
	{
		rfds = orgset;
		nready = select(maxfd+1, &rfds, NULL, NULL, NULL);
		if (nready == -1)
			perr_exit("select");

		if (FD_ISSET(listenfd, &rfds))
		{
			connfd = accept(listenfd, (struct sockaddr*)&clie_addr, &clie_attr_len);
			if (connfd > 0)
			{
				printf("[NEW]client from %s:%d\n", inet_ntop(AF_INET, &clie_addr.sin_addr, client_ip, sizeof(client_ip)),\
						ntohs(clie_addr.sin_port));
				for (i = 0; i < MAX_CLIENT; i++)
				{
					if(client[i].fd == -1)
					{
						client[i].fd = connfd;
						client[i].port = ntohs(clie_addr.sin_port);
						client[i].index = 0;
						client[i].cindex = 0;
						memset(client[i].original, 0, sizeof(client[i].original));
						memset(client[i].ciphertext, 0, sizeof(client[i].ciphertext));
						strcpy(client[i].ip, client_ip);
						break;
					}
				}
			}

			if (i == MAX_CLIENT)
			{
				fputs("[ERROR]too many clients\n", stderr);
				exit(EXIT_FAILURE);
			}

			FD_SET(connfd, &orgset);
			if (connfd > maxfd)
				maxfd = connfd;

			if (i > maxi)
				maxi = i;
			if (--nready == 0)
				continue;
		}

		for (i = 0; i <= maxi; i++)
		{
			if ((sockfd = client[i].fd) == -1)
				continue;
			if (FD_ISSET(sockfd, &rfds))
			{
				if ((retval = read(sockfd, msgbuf, sizeof(msgbuf))) == 0)
				{
					close(sockfd);
					FD_CLR(sockfd, &orgset);
					client[i].fd = -1;
				}
				else if (retval > 0)
				{
					if (strcmp(msgbuf, "quit\n") == 0)
					{
						printf("[BYE]client from %s:%d exit\n", client[i].ip, client[i].port);
						printf("originaltext: %s, len: %d\n", client[i].original, client[i].index);
						printf("ciphertext: %s, len: %d\n", client[i].ciphertext, client[i].cindex);
						//printf("ciphertext: ");
						//for(int ind = 0; ind < strlen(client[i].ciphertext); ind++)
						//	printf("%c", client[i].ciphertext[ind]);
								
						close(sockfd);
						FD_CLR(sockfd, &orgset);
						client[i].fd = -1;
						memset(client[i].original, 0, sizeof(client[i].original));
						memset(client[i].ciphertext, 0, sizeof(client[i].ciphertext));
						fflush(stdin);
					}
					else
					{	
						save_history(msgbuf, client[i].ciphertext, client[i].cindex, retval);
						client[i].cindex = strlen(client[i].ciphertext);

						decryption(msgbuf, message, retval);

						save_history(message, client[i].original, client[i].index, strlen(message));
						client[i].index = strlen(client[i].original);

						printf("[MESSAGE]from %s:%d->%s\n", client[i].ip, client[i].port, message);
						write(client[i].fd, message, sizeof(message));
						memset(msgbuf, 0, sizeof(msgbuf));
					}
				}
				
				if (--nready == 0)
					break;
			}
		}
	}
}
