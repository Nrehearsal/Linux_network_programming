#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(void)
{
	struct sockaddr_in server_addr;
	int sock_fd;
	int ret;
	char buf[BUFSIZ];
	socklen_t serv_addr_len = sizeof(server_addr);

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(6666);

	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_fd < 0)
	{
		perror("sock_fd error");
		exit(-1);
	}

	while (1) {
		fgets(buf, sizeof(buf),  stdin);
		if (strcmp(buf, "quit\n"))
		{

			ret = sendto(sock_fd, buf, sizeof(buf), 0, (struct sockaddr *)&server_addr, serv_addr_len);
			if (ret > 0)
			{
				printf("data send success!\n");
			}
		}
		else
		{

			ret = sendto(sock_fd, buf, sizeof(buf), 0, (struct sockaddr *)&server_addr, serv_addr_len);
			close(sock_fd);
			exit(1);
		}

	}

	close(sock_fd);
}
