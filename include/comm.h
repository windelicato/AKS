#ifndef _COMM_H_
#define _COMM_H_
#include<sys/socket.h>

void *get_in_addr(struct sockaddr *sa);

int get_msg(const char* ip, char* buf);

int send_mesg(const char* ip, char* message);

#endif
