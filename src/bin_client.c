#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>


#include "serial.h"
#include "comm.h"
#include "lightbar_functions.h"
#include "i2c_functions.h"
#include "AKS_errors.h"
#include "configuration.h"

#define MAXBUFLEN 100
#define NUM_BINS 5

#define HAND_REMOVED_WAIT_CYCLES 100000
#define OBJECT_REMOVED_WAIT_CYCLES 1000

extern const char* log_file_path;
extern const char* config_file_path;

char chip0Init[2] = {0xFF,0xFF};

int num_bins = NUM_BINS;
int num_scales = NUM_BINS;

struct scale_list s;
struct network_data *data;
pthread_t* tid;
pthread_attr_t* attr;

//Initialize AKS for operation
void init() {
	log_message(log_file_path,"AKS System Startup");
	sleep(1);
	//Initialize the different AKS systems
	int temp;
	if((i2c_init())<0) {//Updated to handle error reporting internally
		//Restart system or re-attempt initialization?
	}
	if((temp = bins_init(&s,num_bins))<0){
		aks_critical_error(temp);
	}
	if((temp = scales_init(&s))<0) {
		aks_critical_error(temp);
	}

	/*
	//TEST OF LED DRIVER
	printf("REACHED I2C CHIP 0 SETUP\n");
	i2c_setGPIOPins(0,chip0Init);
	i2c_readGPIOPins(0,chip0Init);
	printf("PINS ARE %d and %d \n",chip0Init[0], chip0Init[1]);
	//TEST
	*/
	
	tid = (pthread_t*)malloc(num_scales*(sizeof(pthread_t)));
	attr = (pthread_attr_t*)malloc(num_scales*(sizeof(pthread_attr_t)));

	int i;
	for (i = 0; i<num_bins; ++i) {
		pthread_attr_init(&attr[i]);
		if (pthread_create(&tid[i], &attr[i], picked, &s.scale[i]) != 0) {
			perror("Unable to create thread");//ERROR!
			log_message(log_file_path, "Could not create bin thread");
		}
	}

	// Initialize networking thread
	data = malloc(sizeof(struct network_data));
	network_init(data, 512);
}

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
	init();	
	
	//TEST OF BASIC NETWORK CONTROLLED PICKING
	//Start of the Message Receiving Process
	// RECIEVE PACKET
	char *message_send = malloc(sizeof(char)*9*MAXBUFLEN);
	char *message_recv = malloc(sizeof(char)*9*MAXBUFLEN);

	while (1) {
		
		memset(message_send, '\0', 9*MAXBUFLEN);
		memset(message_recv, '\0', 9*MAXBUFLEN);

	//	get_msg(argv[1], message_recv);
		while(message_recv[0] == '\0') {
			get_msg_recv(data, message_recv);
		}
		strcpy(message_send, message_recv);
		printf("Tokenizing packet...\n");

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
		//End of the Message Receiving Process
			
		
		
		//TEST OF LIGHTBAR OPERATION WITHOUT NETWORK
//		s.scale[0].quantity_needed = 1;
//		enableLightBar(s.scale[0].lightbar);
//		s.scale[1].quantity_needed = 0;
//		disableLightBar(s.scale[1].lightbar);
//		s.scale[2].quantity_needed = 1;
//		enableLightBar(s.scale[2].lightbar);
//		s.scale[3].quantity_needed = 0;
//		disableLightBar(s.scale[3].lightbar);	
//		s.scale[4].quantity_needed = 1;
//		enableLightBar(s.scale[4].lightbar);
		//TEST
		
		/*
		//TEST OF LED DRIVER
		int button_pressed;

		button_pressed = i2c_readGPIOPin(0,1);

		if(button_pressed) {
			
			i2c_setLEDDriverPin(0,0,0);
			i2c_setLEDDriverPin(0,1,0);
		} else {
			i2c_setLEDDriverPin(0,0,255);
			i2c_setLEDDriverPin(0,1,255);	
		}
		*/

		int hand_in_bin_currently[num_bins];
		int scale_pick_status[num_bins];
		int hand_removed_from_bin[num_bins];
		int object_removed_from_bin[num_bins];
		//int i;
		
		for(i = 0; i<num_bins; i++) {
			hand_removed_from_bin[i] = 0;
			object_removed_from_bin[i] = 0;
		}

		//Start of the Picking Process
		while(1){

			//Bin Pick Detection
			
			//Data gathering loops, reads data from devices
			//int hand_in_bin_currently[num_bins];
			//int scale_pick_status[num_bins];
			//int hand_removed_from_bin[num_bins];
			//int i;
			for(i = 0; i<num_bins; i++) {
				//Light Bar Data
				if(check_lightbar_picked(i,&(s.sem_lightbar[i]))==0) {
					hand_in_bin_currently[i] = 1;
				} else {
					hand_in_bin_currently[i] = 0;
				}
				
				//Scale Picking Data
				scale_pick_status[i] = check_scale_picked(i,&(s.sem_weight[i]));
			}
			
			for(i = 0; i<num_bins; i++) {
				//Cycle timer decrement
				if(hand_removed_from_bin[i]>0) {
					hand_removed_from_bin[i]--;
				}
				if(object_removed_from_bin[i]>0) {
					object_removed_from_bin[i]--;
				}

				if((s.scale[i].hand_in_bin == 1)&&(hand_in_bin_currently[i]==0)) {
					//Hand was removed from bin
					hand_removed_from_bin[i] = HAND_REMOVED_WAIT_CYCLES;

					//TEST
					printf("Hand removed from Bin %d\n", i);
				}	
				s.scale[i].hand_in_bin = hand_in_bin_currently[i];

				if((scale_pick_status[i] == 1)) {
					//Scale detected one item being picked
					//Correct Pick

					if(hand_in_bin_currently[i]) {
						detected_pick(i);
					} else {
						object_removed_from_bin[i] = OBJECT_REMOVED_WAIT_CYCLES;
					}

					//TEST
					printf("Object removed from Bin %d\n", i);

				} else if((scale_pick_status[i] == 2)) {
					//Scale detected more than one item being picked
					//Potential error detected, more than one item probably picked

				} else if((scale_pick_status[i] == 3)) {
					//Scale detected less than one item being picked
					//Potential error detected, less than one items probably picked

				} else {
					//Scale detected no pick
					//Do nothing
				}

				if((hand_removed_from_bin[i]>0)&&(object_removed_from_bin[i]>0)) {
					detected_pick(i);
					hand_removed_from_bin[i] = 0;
					object_removed_from_bin[i] = 0;
				}
			}
			//Check if Picking Process is complete
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
		}
		//End of the Picking Process

		printf("Done Picking!!!");
		
		//TEST OF BASIC NETWORK CONTROLLED PICKING
		//Start of the Message Reply Process
		set_msg_send(data, message_send);
		//End of the Message Reply Process
	}


	return 0;
}
