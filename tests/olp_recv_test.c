#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comm.h"

#define MAXBUFLEN 100

int main (int argc, const char *argv[])
{
	char *message_send = malloc(sizeof(char)*9*MAXBUFLEN);
	char *message_recv = malloc(sizeof(char)*9*MAXBUFLEN);

	while (1) {
		memset(message_send, '\0', 9*MAXBUFLEN);
		memset(message_recv, '\0', 9*MAXBUFLEN);

		get_msg(argv[1], message_recv);
		send_msg(argv[1], message_send);
	}
	return 0;
}
