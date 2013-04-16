#ifndef _COMM_H_
#define _COMM_H_
#include<sys/socket.h>
#include<semaphore.h>

struct network_data{
	char *msg_recv;
	char *msg_send;
	sem_t lock_send;
	sem_t lock_recv;
	pthread_t thread_id;
	pthread_attr_t thread_attr;
	int size;
};

void set_msg_send(struct network_data *data, char* buff);
void get_msg_recv(struct network_data *data, char* buff);
int network_init(struct network_data *data, int size);
void *server_daemon(void *arg);
<<<<<<< HEAD
void *sender(void *arg);
=======
>>>>>>> cd8a344e1562cbe025da487986df2a37154e67b3



void *get_in_addr(struct sockaddr *sa);

int get_msg(const char* ip, char* buf);

int send_msg(const char* ip, char* message);

char *olp_send_recv(const char* host, int port, char* message);

#endif
