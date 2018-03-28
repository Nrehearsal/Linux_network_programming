#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

int main(void)
{
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	int sock_fd;
	char message[BUFSIZ];
	int ret;
	socklen_t client_addr_len = sizeof(client_addr);

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(6666);

	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_fd < 0)
	{
		perror("socket failed:");
		exit -1;
	}

	ret = bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (ret < 0)
	{
		perror("bind error");
		exit -1;
	}

	while(1)
	{
		ret = recvfrom(sock_fd, message, sizeof(message), 0, (struct sockaddr *)&client_addr, &client_addr_len);
		if (ret > 0)
		{
			if (!strcmp(message, "quit\n"))
			{
				close(sock_fd);
				printf("server closed\n");
				exit(1);
			}else
			{
				printf("recived data from %s:%d->%s", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), message);
			}
		}
	}	

	close(sock_fd);

	return 0;

}
