#ifndef _COMM_H_
#define _COMM_H_

void *get_in_addr(struct sockaddr *sa);

int get_msg(const char* ip, char* buf);

int send_mesg(const char* ip, char* message);

#endif
