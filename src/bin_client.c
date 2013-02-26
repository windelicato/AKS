#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>


#include "serial.h"
#include "comm.h"

#define MAXBUFLEN 100


int main(int argc, const char *argv[])
{
	FILE *scales[10];
	char *device_path = (char*) malloc(sizeof(char)*MAXBUFLEN);
	int i, num_scales;

	for( i=0; i<10; i++) {
		sprintf(device_path,"/dev/ttyUSB%d",i);
		scales[i] = fopen(device_path,"r");
		if(!scales[i]){
			printf("Unable to open device %d\n",i);
			num_scales = i;
			break;
		}
	}

	pthread_t tid[num_scales];
	pthread_attr_t attr[num_scales];

	for (i = 0; i<num_scales; ++i){
		printf("%d\n",i);
		pthread_attr_init(&attr[i]);
		if (pthread_create(&tid[i], &attr[i], picked, scales[i]) != 0){
			perror("Unable to create thread");
			exit(-1);
		}
	}

	sleep(100000);

	for (i=0; i<num_scales; ++i) {
		pthread_join(tid[i], NULL);
	}

//	while(1) {
//		for( i=0; i<num_scales; i++) {
//			char *message = picked(scales[i]);
//			printf("Picked %s from scale %d\n", message, i);
//		}
//	}

	return 0;
}
