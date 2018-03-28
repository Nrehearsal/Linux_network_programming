#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <arpa/inet.h>

#define SERV_PORT 6666
#define MAX_CLIENT 32
#define KEY "2015032042"

typedef struct client_data{
	struct sockaddr_in clie_addr;
	int connfd;
}client_info;

static void decryption(const char* source, char* dest)
{
	int len = strlen(source);
	int offset;
	int i;
	for(offset = 0; offset < len; offset += 10)
	{
		for (i = 0; i < 10; i++)
		{
			dest[offset + i] = source[offset + i] - KEY[i];
		}
	}
}
void *do_work(void *arg);

int main(void)
{
	struct sockaddr_in serv_addr, clie_addr;
	int lfd, cfd, ret;
	socklen_t clie_addr_len = sizeof(clie_addr);
	pthread_t tid;
	client_info cset;
	char ip[INET_ADDRSTRLEN];
	int i;

	//初始化客户端信息结合
	/*for (i = 0; i < MAX_CLIENT; i++)
	{
		cset[i].connfd = -1;
	}
	*/

	lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd < 0)
	{
		perror("Create socket failed");
		exit(-1);
	}
	int opt = 1;
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));

	//初始化服务端信息
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(SERV_PORT);

	ret = bind(lfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (ret == -1)
	{
		perror("bind failed");
		exit(-1);
	}

	ret = listen(lfd, MAX_CLIENT);
	if (ret == -1)
	{
		perror("listen failed");
		exit(-1);
	}


	while(1)
	{
		cfd = accept(lfd, (struct sockaddr *)&clie_addr, &clie_addr_len);
		printf("CLIENT FROM %s:%d\n", inet_ntop(AF_INET, &clie_addr.sin_addr, ip, sizeof(ip)),\
				ntohs(clie_addr.sin_port));
		cset.connfd = cfd;
		cset.clie_addr = clie_addr;

		pthread_create(&tid, NULL, do_work, (void *)&cset);
		pthread_detach(tid);

		sleep(1);
		bzero(&clie_addr, clie_addr_len);
	}

	close(lfd);
	return 0;
}

void *do_work(void *arg)
{
	client_info *tmp = (client_info *)arg;
	char buf[BUFSIZ];
	char message[BUFSIZ];
	int readn;
	char *client_ip = inet_ntoa(tmp->clie_addr.sin_addr);

	while(1)
	{
		readn = read(tmp->connfd, &buf, sizeof(buf));
		if (readn > 0)
		{
			if (strcmp(buf, "quit\n"))
			{
				decryption(buf, message);
				printf("recive from %s:%d->%s\n", client_ip, ntohs(tmp->clie_addr.sin_port), message);
				write(tmp->connfd, buf, sizeof(buf));
			}
			else
			{
				printf("recive from %s:%d EXIT.\n", client_ip, ntohs(tmp->clie_addr.sin_port));
				pthread_exit(NULL);
			}
		}
		memset(buf, 0, sizeof(message));
		memset(buf, 0, sizeof(buf));
	}
	close(tmp->connfd);
}
