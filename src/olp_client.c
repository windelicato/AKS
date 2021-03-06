#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>

#include "comm.h"
#include "scan.h"

#define MAXBUFLEN 512

struct network_data n;
const char* IP = "134.82.135.15";

int main(int argc, const char *argv[])
{
	char *message_send = malloc(sizeof(char)*9*MAXBUFLEN);
	char *message_recv = malloc(sizeof(char)*9*MAXBUFLEN);
	char *collat = malloc(sizeof(char)*9*MAXBUFLEN);
	char *order = malloc(sizeof(char)*MAXBUFLEN);
	memset(order , '\0', MAXBUFLEN);

//	network_init(&n, MAXBUFLEN);	


	while (1) {

		memset(message_send, '\0', 9*MAXBUFLEN);
		memset(message_recv, '\0', 9*MAXBUFLEN);
		memset(collat , '\0', 9*MAXBUFLEN);


		
		char*	scan = get_scan("/home/aks/scan.txt");
		strcpy(order, scan);

		printf("Requesting parts from STB# %s\n\n",scan);

		collat = get_collat(scan);

		strcat(message_send,"1 ");
		strcat(message_send,scan);
		strcat(message_send," ");
		strcat(message_send,collat);

		printf("packet sent: %s\n",message_send);

//		set_mesg_send(&n, message_send);
		message_recv = 	olp_send_recv(IP, 4950, message_send);
//		get_mesg_recv(&n, message_recv);

		if( message_recv[0] == message_send[0] ){
			printf("Correctly managed !\n");
		} else{
			printf("ERROR!\n");
		}

		printf("\n\n");
/*
*/
	}

	return 0;
}
