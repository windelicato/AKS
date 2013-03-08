#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>


#include "serial.h"
#include "comm.h"
#include "lightbar_functions.h"

#define MAXBUFLEN 100

struct scale_list s;

void print_pick_info(int bin) {
	printf("Bin number %d picked an item\n", bin);
	printf("SKU: %d \n", s.scale[bin].sku);
}

void detected_pick(int bin) {
	if(s.scale[bin].quantity_needed > 0) {
		s.scale[bin].quantity_needed=0;
		print_pick_info(bin);
		disableLightBar(s.scale[bin].lightbar);
	} else {
		printf("Incorrect Pick on Bin: %d \n",bin);
	}
}

int main(int argc, const char *argv[])
{
	sleep(0.1);

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

	// RECIEVE PACKET
	char *message_send = malloc(sizeof(char)*9*MAXBUFLEN);
	char *message_recv = malloc(sizeof(char)*9*MAXBUFLEN);

	while (1) {
		memset(message_send, '\0', 9*MAXBUFLEN);
		memset(message_recv, '\0', 9*MAXBUFLEN);

		get_msg(argv[1], message_recv);

		char tokens[10][12];
		char *next_str;
		strcpy(tokens[0],strtok(message_recv, " "));

		int i, max_token;
		for(i=1; i<10; i++) {
			if ((next_str = strtok(NULL," ")) != NULL){
				strcpy(tokens[i], next_str);
			}
			else {
				break;
			}
		}
		max_token= i;

		int j;
		for(i=2; i<max_token; i++){
			for(j=0; j<num_scales; j++) {
				if(s.scale[j].sku == atoi(tokens[i])){
					s.scale[j].quantity_needed = 1;
					enableLightBar(s.scale[j].lightbar);
				}
			}
		}

//		for (i=0; i<10; i++) {
//			printf("TOKEN %d: %s\n", i, tokens[i]);
//		}

		while(1){
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
				if(check_percent_full(i,&(s.sem_weight[i])) == s.scale[i].percent_full){
				}
				else {
					s.scale[i].percent_full = check_percent_full(i,&(s.sem_weight[i]));
					printf("Bin number %d percent full %f\n",i, s.scale[i].percent_full);
				}

			}
			int done_picking = 1;
			for(i = 0; i<num_scales; i++) {
				if (s.scale[i].quantity_needed > 0) {
					done_picking = 0;
				}
			}

			if(done_picking){
				printf("here\n");
				break;
			}

			sleep(0.25);
		}

		message_send = message_recv;
		send_msg(argv[1], message_send);
	}


	return 0;
}
