#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>


#include "serial.h"
#include "comm.h"

#define MAXBUFLEN 100

struct scale_list s;
int lightbars[] = {0, 1, 2, 3, 4};
int skus[] = {11000000, 11018971, 11073767, 11083039, 11180134};

int main(int argc, const char *argv[])
{
	scales_init(&s, 5);

	int num_scales = open_scales(&s);
	printf("%d\n", num_scales);

	pthread_t tid[num_scales];
	pthread_attr_t attr[num_scales];
	
	int i;
	for (i = 0; i<num_scales; ++i){
		s.scale[i].lightbar = lightbars[i];
		s.scale[i].sku = skus[i];
	}

	for (i = 0; i<num_scales; ++i){
		pthread_attr_init(&attr[i]);
		if (pthread_create(&tid[i], &attr[i], picked, &s.scale[i]) != 0){
			perror("Unable to create thread");
			exit(-1);
		}
	}

	while(1){
		int bin_num = check_picked(&s.sem);
		if(bin_num >= 0) {
			double perc_full = check_percent_full(&s.sem);
			printf("Bin number %d picked an item and is %f percent full\n",bin_num,perc_full);
		}
		sleep(0.5);
	}

	return 0;
}
