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
#define SERV_HOST "127.0.0.2"
#define KEY "2015032042"

static int 
encrypt(const char* source, char * dest)
{
	int len;
	int new_len;
	char *msg;
	int offset;
	int i;

	len = strlen(source);
	for (new_len = len; new_len % (10) != 0; new_len++);
	printf("message len:%d\n", new_len);

	msg = (char *)malloc(new_len);
	memset(msg, 0 ,new_len);
	memcpy(msg, source, len);

	for (offset = 0; offset < new_len; offset += 10)
	{
		for (i = 0; i < 10; i++)
		{
			dest[offset + i] = (msg[offset + i] + KEY[i]);
		}
	}
	printf("offset: %d\n", offset);
	free(msg);
	return offset;
}

int 
main(int c, char **arg)
{
	struct sockaddr_in serv_addr;
	int ret, i;
	int cfd;
	char buf[BUFSIZ];
	char message[BUFSIZ];
	int n;
	int writen;
	if(c < 2)
		exit(1);

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
		printf("sizeof buf %d\n", (int)sizeof(buf));
	}

	while (1)
	{
		fgets(buf, sizeof(buf), stdin);
		if (strcmp(buf, "quit\n"))
		{
			buf[strlen(buf)-1] = '\0';
			//encrypt(buf, message);
			//writen = write(cfd, message, strlen(message));
			ret = encrypt(buf, message);
			writen = write(cfd, message, ret);
			printf("---------------%d\n", writen);
			if (writen < 0)
			{
				fprintf(stderr, "Server has been closed.\n"); break;
				break;
			}
		}
		else
		{
			writen = write(cfd, buf, sizeof(buf));
			if (writen < 0) {
				fprintf(stderr, "Server has been closed.\n");
			} else {
				printf("I will be closed.\n");
				break;
			}
		}

		n = read(cfd, buf, sizeof(buf));
		if (n > 0) {
			printf("[SERVER]return message: %s\n", buf);
			//write(STDOUT_FILENO, buf, strlen(buf));
			//printf("\n");
		}
	}

	close(cfd);
	return 0;
}
