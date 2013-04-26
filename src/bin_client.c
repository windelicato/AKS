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

//If defined will enable some printf statement that are helpful for debugging
#define DEBUG

//The length to use for the message buffer
#define MAXBUFLEN 100

//The number of bins
#define NUM_BINS 5

//The number of "pseudo-bins", bins that do not have a scale or lightbar and only use an LED and button
#define NUM_PSEUDO_BINS 1

//These are the numbers for the restocking indicator LEDs, they map to LED Driver pins in "led_functions.c"
#define RED_RESTOCK_LED 6
#define YELLOW_RESTOCK_LED 7
#define GREEN_RESTOCK_LED 8

//The number of cycles that a button must be pressed down for it to count as a button press
#define BUTTON_PRESSED_WAIT_CYCLES 30

//The maximum number of loop cycles to wait for a scale to confirm a pick after a lightbar detects a hand being removed from a bin 
#define HAND_REMOVED_WAIT_CYCLES 100000

//The maxium number of loop cycles to wait for a lightbar to confirm a pick after a scale detects an object being removed
//NOTE: Currently not used, scales are
#define OBJECT_REMOVED_WAIT_CYCLES 1000

extern const char* log_file_path;
extern const char* config_file_path;

char chip0Init[2] = {0xFF,0xFF};

//Variables should be replaced with a scale list variable eventually
int num_bins;
int num_scales;
int num_pseudo_scales;

struct scale_list s;
struct network_data *data;
pthread_t* tid;
pthread_attr_t* attr;

//Initialize AKS for operation
void init() {
	#ifdef DEBUG
	printf("Starting initialization...\n");
	#endif

	num_bins = NUM_BINS;
	num_scales = NUM_BINS;
	num_pseudo_scales = NUM_PSEUDO_BINS;

	log_message(log_file_path,"AKS System Startup");
	sleep(1);
	//Initialize the different AKS systems
	int temp;
	
	#ifdef DEBUG
	//TEST
	printf("Starting device initialization...\n");
	#endif
	
	if((i2c_devices_init())<0) {//Updated to handle error reporting internally
		//Restart system or re-attempt initialization?
	}

	if((temp = bins_init(&s,num_bins,num_pseudo_scales))<0){
		aks_critical_error(temp);
	}
	if((temp = scales_init(&s))<0) {
		aks_critical_error(temp);
	}

	#ifdef DEBUG
	//TEST
	printf("Finished device initialization.\n");
	#endif
	
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

	#ifdef DEBUG
	printf("Finished initialization.\n");
	#endif
}

//FUNCTION IS NOT USED AND WILL NOT WORK
//Starts the picking process
void start_picking(int* bin_pick_status) {
	int i;
	for(i=0;i<sizeof(s.scale);i++) {
		if(s.scale[i].quantity_needed==0) {
			bin_pick_status[i] = 0;//None needed, no bin activity
			disableLightBar(s.scale[i].lightbar);
		} else if(s.scale[i].quantity_needed==1) {
			bin_pick_status[i] = 1;//One item needed, no bin activity
			enableLightBar(s.scale[i].lightbar);
		} else if(s.scale[i].quantity_needed>=2) {
			bin_pick_status[i] = 2;//More than one item neeed, no bin activity
			enableLightBar(s.scale[i].lightbar);
		} else {
			//Error
		}
	}
}

void print_pick_info(int bin) {
	printf("Bin number %d picked an item\n", bin);
	printf("SKU: %d \n", s.scale[bin].sku);
}

//Takes appropriate action when a pick is detected during the picking process
void detected_pick(int bin, int* nums, int* ignore_state, int* items_needed) {
	if(ignore_state[bin] == 1) {
		//Ignore picks from this bin

		return;
	}

	return;

	if(items_needed[bin] == nums[bin]) {
		//Picking complete
		items_needed[bin] -= nums[bin];
		
		//PICKING COMPLETE
		//Disable LightBar
		disableLightBar(s.scale[bin].lightbar);

		//Pulse LED?
		//Network Message?
		
		//TEST
		printf("Picking complete for this bin\n");
		print_pick_info(bin);

	} else if(items_needed[bin] > nums[bin]) {
		//Picking partially complete, more items to pick
		items_needed[bin] -= nums[bin];
		
		//Pulse LightBar?
		//Pulse LED?
		//Network Message?

		//TEST
		printf("Picking partially complete\n");
		print_pick_info(bin);

	} else if(items_needed[bin] < nums[bin]) {
		//Error, more items picked than needed
		
		//TEST
		printf("Error, incorrect pick\n");

		if(items_needed[bin]>0) {
			//If some of the picked items were needed
			
			//PICKING COMPLETE
			//Disable LightBar
			disableLightBar(s.scale[bin].lightbar);
			
			//Pulse LED?
			//Network Message?

			//TEST
			printf("Too many items were picked...\n");
			print_pick_info(bin);
		}

		items_needed[bin] -= nums[bin];


		//Light LED
		setLED(s.scale[bin].led, 255);

		//Log: Incorrect pick (info?)?
	}

	#ifdef DEBUG
	printf("Finished detecting a pick.\n");
	#endif
}

//React to a button press for a certain bin
void bin_button_press(int bin, int* ignore_state, int* items_needed) {
	if((items_needed[bin] > 0)&&(ignore_state[bin]==0)) {
		//Picking was not complete on bin, button press forces program to assume picking complete
		items_needed[bin] = 0;

		//Disable LightBar
		disableLightBar(s.scale[bin].lightbar);

		//Pulse LED?
		//Network Message?
	} else {
		//Picking was either complete or the bin was in an error state or the bin already has been ignored with a button press
		items_needed[bin] = 0;
		ignore_state[bin] = 1;

		//Turn off LED
		setLED(s.scale[bin].led, 0);

		//Log: Error overriden with button override?
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
		
		#ifdef DEBUG
		printf("Waiting for network message...\n");
		#endif

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

		////////////////////////////////////////////
		//                                        //
		//  ************************************  //
		//  End of the Message Receiving Process  //
		//  ************************************  //
		//                                        //
		////////////////////////////////////////////



		////////////////////////////////////
		//                                //  
		//  \/\/\/\/\/\/\/\/\/\/\/\/\/\/  //  
		//  Start of the Picking Process  //  
		//  \/\/\/\/\/\/\/\/\/\/\/\/\/\/  //  
		//                                //  
		////////////////////////////////////

		///////////////////////////////
		// Picking Process Variables //
		///////////////////////////////

		#ifdef DEBUG
		printf("Starting the Picking Process...\n");
		printf("Number of Scales: %d, Number of Pseudo-Scales: %d\n",s.num_scales,s.num_pseudo_scales);
		#endif 


		int ignore_bin[num_bins];//0: normal operation, 1: ignore bin
		int bin_items_needed[s.num_scales+s.num_pseudo_scales];//(needed in pseudo-scales)
		int scale_last_num_items_removed[num_bins];

		int hand_in_bin_currently[num_bins];//0 or 1 boolean values
		int scale_pick_status[num_bins];//0,1,2,3 for no pick, 1 item picked, more than one item picked, and less than one item picked
		double scale_last_percent_full[num_bins];
		double lowest_percent_full = 100.0;

		int button_pressed[s.num_scales+s.num_pseudo_scales];//decreasing timers (needed in pseudo-scales)
		int hand_removed_from_bin[num_bins];//decreasing timers
		int object_removed_from_bin[num_bins];//decreasing timers


		int refresh_timer = 0;
		int refresh_interval = 50;
		

		/////////////////////////////////////////////////
		// Picking Process Initialization of Variables //
		/////////////////////////////////////////////////

		for(i = 0; i<num_bins; i++) {

			ignore_bin[i] = 0;
			
			if((s.scale[i].sku==-1)&&(s.scale[i].quantity_needed<=0)) {
				bin_items_needed[i] = 1;
			} else {
				bin_items_needed[i] = s.scale[i].quantity_needed;
			}

			//TEMPORARY
			//ACTIVATE APPROPRIATE BINS
			if((s.scale[i].quantity_needed>0)||(s.scale[i].sku==-1)){
				enableLightBar(s.scale[i].lightbar);
			} else {
				disableLightBar(s.scale[i].lightbar);
			}

			scale_last_num_items_removed[i] = 0;

			button_pressed[num_bins] = 0;
			hand_removed_from_bin[i] = 0;
			object_removed_from_bin[i] = 0;
		}

		for(i = 0; i<s.num_pseudo_scales; i++) {
			
			if(s.scale[i+s.num_scales].sku==-1) {
				bin_items_needed[i+s.num_scales] = 1;
			} else {
				bin_items_needed[i+s.num_scales] = s.scale[i+s.num_scales].quantity_needed;
			}
			button_pressed[i+s.num_scales] = 0;

			//TEST
			printf("Pseudo-bin %d has sku %d and quantity_needed %d.\n",i,s.scale[i+s.num_scales].sku,s.scale[i+s.num_scales].quantity_needed);

			//ACTIVATE APPROPRIATE BINS
			if((s.scale[i+s.num_scales].quantity_needed>0)||(s.scale[i+s.num_scales].sku==-1)) {
				setLED(s.scale[i+s.num_scales].led,255);
			} else {
				//May want to disable LED otherwise
			}

			//TEST
			printf("Pseudo-bin %d has LED %d and button %d.\n",i, s.scale[i+s.num_scales].led, s.scale[i+s.num_scales].button);

		}


		///////////////////////////////
		// Picking Process Main Loop //
		///////////////////////////////
		
		while(1){
			
			if(refresh_timer >= refresh_interval) {
				refresh_timer = 0;
				button_refresh();
				
				for(i=0;i<s.num_scales;i++) {
					if(bin_items_needed[i] <= 0) {
						disableLightBar(s.scale[i].lightbar);
					} else {
						enableLightBar(s.scale[i].lightbar);
					}
				}

			} else {
				refresh_timer++;
			}

			//Bin Pick Detection
			
			//Data gathering loops, reads data from devices
			//int hand_in_bin_currently[num_bins];
			//int scale_pick_status[num_bins];
			//int hand_removed_from_bin[num_bins];
			//int i;

			//Data Collection
			for(i = 0; i<num_bins; i++) {
				//Light Bar Data
				if(check_lightbar_picked(i,&(s.sem_lightbar[i]))==0) {
					hand_in_bin_currently[i] = 1;
					//printf("HAND ENTERED LIGHTBAR %d\n",s.scale[i].lightbar);
				} else {
					hand_in_bin_currently[i] = 0;
				}
				
				//Scale Picking Data
				check_scale_pick_data(i,&(s.sem_weight[i]),&(scale_pick_status[i]),&(scale_last_num_items_removed[i]),&(scale_last_percent_full[i]));
			}
			
			//State Change Decision Making
			for(i = 0; i<num_bins; i++) {

				//Cycle timer decrement
				if(hand_removed_from_bin[i]>0) {
					hand_removed_from_bin[i]--;
				}
				if(object_removed_from_bin[i]>0) {
					object_removed_from_bin[i]--;
				}
				if(button_pressed[i]>0) {
					button_pressed[i]--;
				}
				
				//Button Data Processing
				if((getButtonState(s.scale[i].button))==1) {
					if(button_pressed[i]>0) {
						//Do nothing
					} else {
						button_pressed[i] = BUTTON_PRESSED_WAIT_CYCLES;
					}
				} else {
					if(button_pressed[i]>(BUTTON_PRESSED_WAIT_CYCLES/6)) {
						button_pressed[i] -= (BUTTON_PRESSED_WAIT_CYCLES/6);
					} else {
						button_pressed[i] = 0;
					}
				}

				if(button_pressed[i]==1) {

					//TEST
					printf("This is button #%d\n",s.scale[i].button);

					bin_button_press(i, ignore_bin, bin_items_needed);
				}

				//Light Bar Data Processing
				if(1) {
				//if((s.scale[i].hand_in_bin == 1)&&(hand_in_bin_currently[i]==0)) {
					//Hand was removed from bin
					hand_removed_from_bin[i] = HAND_REMOVED_WAIT_CYCLES;
					//printf("HAND REMOVED from %d\n", s.scale[i].lightbar);
				}	
				s.scale[i].hand_in_bin = hand_in_bin_currently[i];

				//Scale Data Processing
				if((s.scale[i].mode==1)) {
					//Ignore the scale
				} else if((scale_pick_status[i] == 1)) {
					//Scale detected one item being picked
					
					object_removed_from_bin[i] = OBJECT_REMOVED_WAIT_CYCLES;

				} else if((scale_pick_status[i] == 2)) {
					//Scale detected more than one item being picked
					
					object_removed_from_bin[i] = OBJECT_REMOVED_WAIT_CYCLES;

				} else if((scale_pick_status[i] == 3)) {
					//Scale detected less than one item being picked
					
					//object_removed_from_bin[i] = OBJECT_REMOVED_WAIT_CYCLES;

				} else if((scale_pick_status[i] == 4)) {
					//Scale detected weight being added, possibly an item being replaced into the bin
					//Clear error conditions if they exist?
					
					//TEST
					printf("Scale detect weight being added (%d)\n",i);

					//MOVE TO ITS OWN FUNCTION?
					if(bin_items_needed[i]<0) {
						//Error condition
						if(bin_items_needed[i]<scale_last_num_items_removed[i]) {
							//Clear the error
							bin_items_needed[i] = 0;
							setLED(s.scale[i].led,0);
						} else {
							bin_items_needed[i] += scale_last_num_items_removed[i];
						}
					} else {
						//Do nothing, adding weight to a bin does not effect a scale that has not had an incorrect pick
					}
					//----------------------------

				} else {
					//Scale detected no pick
					//Do nothing
				}

				//Pick Decision
				if((s.scale[i].mode==1)&&(0)&&(hand_removed_from_bin[i]>0)) {
					if(bin_items_needed[i]>1) {
						scale_last_num_items_removed[i] = bin_items_needed[i];
						detected_pick(i, scale_last_num_items_removed, ignore_bin, bin_items_needed);
					} else if(0) {
						scale_last_num_items_removed[i] = 1;
						detected_pick(i, scale_last_num_items_removed, ignore_bin, bin_items_needed);
					}
					hand_removed_from_bin[i] = 0;
					object_removed_from_bin[i] = 0;

					//TEST
					printf("Mode 1 Bin (ignoring scales) detected a pick. Bin %d\n",i);

				} else if((hand_removed_from_bin[i]>0)&&(object_removed_from_bin[i]>0)) {
					detected_pick(i, scale_last_num_items_removed, ignore_bin, bin_items_needed);
					hand_removed_from_bin[i] = 0;
					object_removed_from_bin[i] = 0;
				}
			}
			
			for(i = 0; i<s.num_pseudo_scales;i++) {
				if(button_pressed[i+s.num_scales] > 0) {
					button_pressed[i+s.num_scales]--;
				}

				if(getButtonState(s.scale[i+s.num_scales].button)==1) {
					if(button_pressed[i+s.num_scales] > 0) {
						//Do nothing, let current timer tick
					} else {
						//No timer running for this press? Start one.
						button_pressed[i+s.num_scales] = BUTTON_PRESSED_WAIT_CYCLES;
					}
				} else {
					if(button_pressed[i+s.num_scales]>(BUTTON_PRESSED_WAIT_CYCLES/5)) {
						button_pressed[i+s.num_scales] -= (BUTTON_PRESSED_WAIT_CYCLES/5);
					} else {
						button_pressed[i+s.num_scales] = 0;
					}
				}

				if(button_pressed[i+s.num_scales] == 1) {
					//Cannot use normal button function for pseudo-scales
					
					//TEST
					printf("Pseudo-Bin %d picking complete, button %d pressed.\n", i, s.scale[i+s.num_scales]);

					//If items were needed, consider them picked
					if(bin_items_needed[i+s.num_scales]>0) {
						//Pick complete for this bin
						bin_items_needed[i+s.num_scales] = 0;
						setLED(s.scale[i+s.num_scales].led,0);
					}
				}
			}
			

			//Check if Picking Process is complete
			int done_picking = 1;
			for(i = 0; i<(s.num_scales+s.num_pseudo_scales); i++) {

				if (bin_items_needed[i] > 0) {
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

		for(i = 0;i<s.num_scales;i++) {
			disableLightBar(s.scale[i].lightbar);
		}
		
		//TEST OF BASIC NETWORK CONTROLLED PICKING
		//Start of the Message Reply Process
		set_msg_send(data, message_send);
		//End of the Message Reply Process

		// Do not let operator scan until previous pick complete 
		sem_wait(&data->lock_recv);
		data->msg_recv[0] = '\0';
		sem_post(&data->lock_recv);

	}


	return 0;
}
