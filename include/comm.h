#ifndef _COMM_H_
#define _COMM_H_
#include<sys/socket.h>
#include<semaphore.h>

struct network_data{
	char msg_recv[1000];
	char msg_send[1000];
	sem_t lock_send;
	sem_t lock_recv;
};

void set_mesg_send(struct network_data *data, char* buff);
void get_mesg_send(struct network_data *data, char* buff);
int network_init();
void *server_daemon(void *arg);



void *get_in_addr(struct sockaddr *sa);

int get_msg(const char* ip, char* buf);

int send_mesg(const char* ip, char* message);

#endif
