#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>

#define SERV_PORT 6666
#define SERV_HOST "127.0.0.2"

int main(int c, char **arg)
{
	struct sockaddr_in serv_addr;
	int ret, i;
	int cfd;
	char buf[BUFSIZ];
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
		printf("sizeof buf %d\n", sizeof(buf));
	}

	while (1)
	{
		fgets(buf, sizeof(buf), stdin);
		if (strcmp(buf, "quit\n"))
		{
			writen = write(cfd, buf, strlen(buf));
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
			printf("return data %s", buf);
		}
	}

	close(cfd);
	return 0;
}
