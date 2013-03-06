#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>

#include "comm.h"
#include "scan.h"

#define MAXBUFLEN 100

int main(int argc, const char *argv[])
{
	/*char* message = malloc(sizeof(char)*20);*/
	printf("**************************\n");
	printf("*  	WELCOME TO 	 *\n");
	printf("*    MY FAKE OLP TEST    *\n");
	printf("*************************\n\n");

	char * scan = get_scan("/home/aks/scan.txt");
	printf("%s\n",scan);

	get_collat(scan);


	//	printf("\nrequest > ");
	//	fgets(message, MAXBUFLEN, stdin);
	//	printf(" requesting pick...\n");

	//	send_msg(argv[1], message);
	//	get_msg(argv[1], buf);
	//	if( buf[0] == message[0] ){
	//		printf("Correct pick!\n");
	//		printf("Picked: %s\n",buf);
	//	} else{
	//		printf("Incorrect pick!\n");
	//		printf("Picked: %s\n",buf);
	//	}

	//	printf("\n\n");

	
	return 0;
}
