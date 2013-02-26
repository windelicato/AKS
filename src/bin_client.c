#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#include "serial.h"
#include "comm.h"

#define MAXBUFLEN 100


int main(int argc, const char *argv[])
{
	FILE *file = fopen("/dev/ttyUSB0","r");
	if(!file){
		perror("Unable to open device");
		exit(-1);
	}

	char * buf = (char*)malloc(sizeof(char)*MAXBUFLEN);
	while(1) {
		get_msg(argv[1], buf);
		printf("Pick a %s\n",buf);

		char *message = picked(file);
		send_msg(argv[1], message);
	}

//	close(file);	
	return 0;
}
