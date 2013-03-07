#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>


#include "serial.h"
#include "comm.h"

#define MAXBUFLEN 100

struct scale_list s;

void print_pick_info(int bin) {
	printf("Bin number %d picked an item and is %f percent full\n", bin, check_percent_full(bin, &(s.sem_weight[bin])));
	printf("SKU: %d \n", s.scale[bin].sku);
}

void detected_pick(int bin) {
	if(s.scale[bin].quantity_needed > 0) {
		//s.scale[bin].quantity_needed--;
		print_pick_info(bin);
	} else {
		printf("Incorrect Pick on Bin: %d \n",bin);
	}
}

int main(int argc, const char *argv[])
{
	scales_init(&s, 5);

	int num_scales = open_scales(&s);
	printf("%d\n", num_scales);

	pthread_t tid[num_scales];
	pthread_attr_t attr[num_scales];
	
	int i;
	for (i = 0; i<num_scales; ++i){
		pthread_attr_init(&attr[i]);
		if (pthread_create(&tid[i], &attr[i], picked, &s.scale[i]) != 0){
			perror("Unable to create thread");
			exit(-1);
		}
	}

	while(1){
		int i;
		for(i = 0; i<num_scales; i++) {
			if(check_lightbar_picked(i,&(s.sem_lightbar[i]))==0) {
				if(s.scale[i].hand_in_bin == 0) {
					s.scale[i].hand_in_bin = 1;
				} else {
					//Do nothing
				}
			} else {
				if(s.scale[i].hand_in_bin == 1) {
					detected_pick(i);
					s.scale[i].hand_in_bin = 0;
				} else {
					//Do nothing
				}
			}
		}
		sleep(0.25);
	}

	return 0;
}
