#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>

#ifdef BUFSIZ

#undef BUFSIZ
#define BUFSIZ 1024

#endif

#define SERV_PORT 6666
#define KEY "2015032042"

static void encrypt(const char* source, char * dest)
{
	int len;
	int new_len;
	char *msg;
	int offset;
	int i;

	len = strlen(source);
	for (new_len = len; new_len % (10) != 0; new_len++);
	//printf("message len:%d\n", new_len);

	msg = (char *)malloc(new_len);
	memset(msg, 0 ,new_len);
	memcpy(msg, source, len);

	for (offset = 0; offset < new_len; offset += 10)
	{
		for (i = 0; i < 10; i++)
		{
			dest[offset + i] = msg[offset + i] + KEY[i];
		}
	}
	free(msg);
}

int main(int c, char **arg)
{
	if (c < 2)
	{
		printf("Usage: %s hostname\n", arg[0]);
		exit(1);
	}
	struct sockaddr_in serv_addr;
	int ret, i;
	int cfd;
	char buf[BUFSIZ];
	char message[BUFSIZ];
	int writen;

	cfd = socket(AF_INET, SOCK_STREAM, 0);
	if (cfd < 0)
	{
		perror("socket failed");
		exit(-1);
	}

	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	serv_addr.sin_addr.s_addr = inet_addr(arg[1]);

	ret = connect(cfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (ret < 0)
	{
		perror("connect failure");
		exit(-1);
	}
	else
	{
		printf("CONNECTED SERVER SUCCESS!\n");
	}

	while (1)
	{
		fgets(buf, sizeof(buf), stdin);
		if (strcmp(buf, "quit\n"))
		{
			buf[strlen(buf) - 1] = '\0';

			//printf("buf:%s", buf);
			//printf("buf len:%d", strlen(buf));

			encrypt(buf, message);
			//printf("%s\n", message);
			//printf("%d\n", strlen(message));
			printf("After encryption: %s\n", message);
			writen = write(cfd, message, sizeof(message));
			if (writen < 0)
			{
				fprintf(stderr, "Server has been closed.\n"); break;
				break;
			}
			memset(message, 0, sizeof(message));
		}
		else
		{
			writen = write(cfd, buf, sizeof(buf));
			if (writen < 0)
				fprintf(stderr, "Server has been closed.\n");
			else
				printf("I will be closed.\n");
			break;
		}
	}

	close(cfd);
	return 0;
}
