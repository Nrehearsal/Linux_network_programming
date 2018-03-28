#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <signal.h>

#define SERV_PORT 6666

void do_sigchild(int num)
{
	int status;
	while (waitpid(0, &status, WNOHANG) > 0)
	{
		if (WIFEXITED(status))
			printf("normal termination, exit status %d\n", WEXITSTATUS(status));
		else if (WIFSIGNALED(status))
			printf("abnormal termination, signal status %d\n",WTERMSIG(status));
	}
}

int main(void)
{
	struct sockaddr_in serv_addr, clie_addr;
	socklen_t clie_addr_len = sizeof(clie_addr);
	pid_t pid;
	int lfd, cfd;
	char buf[BUFSIZ];
	char client_ip[INET_ADDRSTRLEN];
	uint16_t client_port;
	int ret;
	int readn;
	int i;
	struct sigaction newact;
	newact.sa_handler = do_sigchild;
	sigemptyset(&newact.sa_mask);
	newact.sa_flags = 0;
	sigaction(SIGCHLD, &newact, NULL);

	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(SERV_PORT);

	lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd < 0) {
		perror("socket fail");
		exit(-1);
	}

	int opt = 1;
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	ret = bind(lfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (ret < 0)
	{
		perror("bind fail");
		exit(-1);
	}
	ret = listen(lfd, 128);
	if (ret > 0)
	{
		perror("listen fail");
		exit(-1);
	}

	printf("SERVER is ready for accpeting CLIENT.\n");

	while (1)
	{
		cfd = accept(lfd, (struct sockaddr *)&clie_addr, &clie_addr_len);
		if (cfd > 0)
		{
			pid = fork();

			if (pid == 0)
			{
				close(lfd);

				inet_ntop(AF_INET, &clie_addr.sin_addr, client_ip, sizeof(client_ip));
				client_port = ntohs(clie_addr.sin_port);

				printf("CLIENT from %s:%d CONNECTED.\n", client_ip, client_port);
				while (1)
				{
					readn = read(cfd, buf, sizeof(buf));
					if (readn <= 0)
					{
						printf("CLINET from %s:%d CLOSED.\n", client_ip, client_port);
						break;
					}

					if (strcmp(buf, "quit\n"))
					{
						printf("receive message from %s:%d = %s", client_ip, client_port, buf);
					}
					else
					{
						printf("CLINET from %s:%d EXIT.\n", client_ip, client_port);
						break;
					}
					memset(buf, '\0' , sizeof(buf));
				}
				close(cfd);
				return 0; 
			}
			else if (pid > 0)
			{
				close(cfd);
			}
			else
			{
				perror("fork failed");
			}
		}
	}
}
