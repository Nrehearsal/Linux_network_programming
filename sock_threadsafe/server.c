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
#ifdef BUFSIZ

#undef BUFSIZ
#define BUFSIZ 1024

#endif

typedef struct client_data{
	struct sockaddr_in clie_addr;
	int connfd;
}client_info;

static void decryption(const char* source, char* dest);

void *do_work(void *arg);
pthread_key_t key;
pthread_once_t once = PTHREAD_ONCE_INIT;

static void 
destructor(void *ptr)
{
	free(ptr);	
}

static void 
creatkey_once(void)
{
	pthread_key_create(&key, destructor);
}

static void client_handle(int sockfd, struct sockaddr_in client_addr);

int 
main(void)
{
	struct sockaddr_in serv_addr, clie_addr;
	int lfd, cfd, ret;
	socklen_t clie_addr_len = sizeof(clie_addr);
	pthread_t tid;
	client_info cset;
	char ip[INET_ADDRSTRLEN];
	int i;

	lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd < 0)
	{
		perror("Create socket failed");
		exit(-1);
	}
	int opt = 1;
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));

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
		printf("[NEW]CLIENT FROM %s:%d\n",\
				inet_ntop(AF_INET, &clie_addr.sin_addr, ip, sizeof(ip)),\
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

static void 
decryption(const char* source, char* dest)
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

static void 
save_history(const char *message, char (*history)[BUFSIZ], const int index)
{
	memcpy(history[index], message, BUFSIZ);
}

static void
client_handle(int sockfd, struct sockaddr_in client_addr)
{
	char buf[BUFSIZ];
	char message[BUFSIZ];
	int readn;
	char *client_ip = inet_ntoa(client_addr.sin_addr);
	int client_port = ntohs(client_addr.sin_port);
	char clie_origin[20][BUFSIZ];
	char clie_cipher[20][BUFSIZ];
	int i;

	pthread_once(&once, creatkey_once);
	int *index = (int *)pthread_getspecific(key);

	if (index == NULL)
	{
		index = (int *)malloc(sizeof(int));
		pthread_setspecific(key, index);
		*index = 0;
	}


	while(1)
	{
		readn = read(sockfd, &buf, sizeof(buf));
		if (readn > 0)
		{
			if (strcmp(buf, "quit\n"))
			{
				decryption(buf, message);
				printf("[MESSAGE]recive from %s:%d->%s\n", client_ip, client_port, message);
				
				write(sockfd, buf, sizeof(buf));
				save_history(message, clie_origin, *index);
				save_history(buf, clie_cipher, *index);
				(*index)++;
			}
			else
			{
				save_history(buf, clie_origin, *index);
				save_history(buf, clie_cipher, *index);
				(*index)++;
				printf("[EXITED]client from %s:%d EXIT.\n", client_ip, client_port);
				
				printf("[HISTORY]thread %ld's cipher text history: ", pthread_self());
				for (i = 0; i < *index; i++)
					printf("[%d]%s ", i+1, clie_cipher[i]);

				printf("[HISTORY]thread %ld's original texthistory: ", pthread_self());
				for (i = 0; i < *index; i++)
					printf("[%d]%s ", i+1, clie_origin[i]);
				
				return;
			}
		}
		memset(buf, 0, sizeof(message));
		memset(buf, 0, sizeof(buf));
	}

}

void *
do_work(void *arg)
{
	client_info *tmp = (client_info *)arg;
	client_handle(tmp->connfd, tmp->clie_addr);
	pthread_exit(NULL);
}
