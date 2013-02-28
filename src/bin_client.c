#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>


#include "serial.h"
#include "comm.h"

#define MAXBUFLEN 100

struct scale_list s;

int main(int argc, const char *argv[])
{
	scales_init(&s, 10);

	char *device_path = (char*) malloc(sizeof(char)*MAXBUFLEN);
	int i, num_scales;

	for( i=0; i<10; i++) {
		sprintf(device_path,"/dev/ttyUSB%d",i);
		s.scale[i].fid = fopen(device_path,"r");
		s.scale[i].id  = i;
		if(s.scale[i].fid == 0){
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
		if (pthread_create(&tid[i], &attr[i], picked, &s.scale[i]) != 0){
			perror("Unable to create thread");
			exit(-1);
		}
	}

	while(1){
		int bin_num = check_picked(&s.sem);
		if(bin_num >= 0) {
			printf("Bin number %d picked an item \n",bin_num);
		}
	}

	return 0;
}
