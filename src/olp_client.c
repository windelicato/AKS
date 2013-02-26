#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comm.h"

#define MAXBUFLEN 100

int main(int argc, const char *argv[])
{
	/*char* message = malloc(sizeof(char)*20);*/
	printf("**************************\n");
	printf("*  	WELCOME TO 	 *\n");
	printf("*    MY FAKE OLP TEST    *\n");
	printf("*************************\n\n");

	char * buf = (char*)malloc(sizeof(char)*MAXBUFLEN);
	char message[MAXBUFLEN];

	while(1) {

		printf("Request the following items to be picked: \n");
		printf(" 	1) Paper\n");
		printf(" 	2) Pamphlet\n");
		printf(" 	3) Power Cable\n");
		printf(" 	4) Remote\n");
		printf(" 	5) Component Cable\n\n");
		printf("\nrequest > ");
		fgets(message, MAXBUFLEN, stdin);
		printf(" requesting pick...\n");

		send_msg(argv[1], message);
		get_msg(argv[1], buf);
		if( buf[0] == message[0] ){
			printf("Correct pick!\n");
			printf("Picked: %s\n",buf);
		} else{
			printf("Incorrect pick!\n");
			printf("Picked: %s\n",buf);
		}

		printf("\n\n");

	}
	
	return 0;
}
