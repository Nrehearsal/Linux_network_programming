#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include "../include/client_event.h"

#define SERV_PORT 8080
#define SERV_HOST "127.0.0.1"

int main(void)
{
	int cfd, ret;
	pid_t my_pid;
	meg my_info;

	//创建套接字
	cfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serv_addr;

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET, SERV_HOST, &serv_addr.sin_addr.s_addr);

	//链接客户端
	ret = connect(cfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (ret < 0)
	{
		perror("Connect error:");
		exit(1);
	}
	
	while(1)
	{
		my_info.event = 1;
		my_info.my_pid = getpid();
		ret = write(cfd, &my_info, sizeof(my_info));
		
		if (ret < 0)
		{
			fprintf(stdin, "write failed\n");
		}
	}

	close(cfd);

	return 0;
}
