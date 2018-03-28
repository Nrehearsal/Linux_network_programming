#ifndef __CLIENT_EVENT_H__
#define __CLIENT_EVENT_H__

typedef struct message meg;
struct message
{	
	int event;
	pid_t my_pid;
};

void client_boot(const int sockfd);
void client_keep(const int sockfd);
void client_exception(const int sockfd);
void client_dead(const int sockfd);

#endif
