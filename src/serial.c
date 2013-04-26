#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include "lightbar_functions.h"
#include "led_functions.h"
#include "i2c_functions.h"
#include "serial.h"
#include "AKS_errors.h"

#define WEIGHTS 	5
#define AVGS 		30
#define MAXBUFLEN 100
#define DIFF 		0.01
#define W_POWER 	0.23
#define W_REMOTE 	0.33
#define W_PAMPHLET 	0.15
#define W_PAPER 	0.02
#define W_COMP 		0.43
#define ERROR 		0.01
#define STABLE_INTERVAL	25 
#define W_EMPTY  	0.05

#define W_BIN 3.35

#define OBJECT_WEIGHT_ACCEPTABLE_ERROR 0.05

#define RED_RESTOCK_LED 7
#define YELLOW_RESTOCK_LED 6
#define GREEN_RESTOCK_LED 8

#define RED_THRESHOLD 10.0
#define YELLOW_THRESHOLD 50.0

#define STABLE_WAIT_CYCLES 15

int def_buttons[] = {0,1,2,4,3,5}; //(needed for pseudo-scales)
int def_leds[] = {0,1,2,4,3,5};//(needed for pseudo-scales)
int def_lightbars[] = {0, 1, 2, 3, 4};
int def_scales[] = {0, 1, 2, 3, 4};
int def_skus[] = {11000000, 11018971, 11073767, 11083039, 11180134, -1};//(needed for pseudo-scales)
int def_modes[] = {1,0,0,0,0};
int def_use_weights[] = {1,1,1,1,1};
double def_weights[] = {W_PAPER,W_PAMPHLET,W_COMP,W_POWER,W_REMOTE};

int* buttons = def_buttons;
int* leds = def_leds;
int* lightbars = def_lightbars;
int* scales = def_scales;
int* skus = def_skus;
int* modes = def_modes;
int* use_weights = def_use_weights;
double* weights = def_weights;

int* 	last_picked_lightbar;//Array of ints for each bin, like last_picked
int*    last_picked;//Array of ints for each bin, bins set value when picked
int* 	last_picked_objects;//Array of ints to indicate how many objects were picked in a single pick
double* last_percent_full;//Array of percent fulls for each bin
double lowest_percent_full = 100.0;


//Sets 
void check_scale_pick_data(int bin, sem_t* lock, int* state, int* objects, double* percent_data) {
	sem_wait(lock);
	state[0] = last_picked[bin];
	last_picked[bin] = 0;
	objects[0] = last_picked_objects[bin];
	last_picked_objects[bin] = 0;
	percent_data[0] = last_percent_full[bin];
	sem_post(lock);
}

int check_scale_picked(int bin, sem_t* lock){ 
	int temp;
	sem_wait(lock);
	temp = last_picked[bin];
	last_picked[bin] = -1;
	sem_post(lock);
	return temp;
}

int check_lightbar_picked(int bin, sem_t* lock) {
	int temp;
	sem_wait(lock);
	temp = last_picked_lightbar[bin];
	//last_picked_lightbar[bin] = -1;
	sem_post(lock);
	return temp;
}

double check_scale_percent_full(int bin, sem_t* lock){ 
	int temp;
	sem_wait(lock);
	temp = last_percent_full[bin];
	//last_percent_full[bin] = -1;
	sem_post(lock);
	return temp;
}

void set_weight_picked(sem_t* lock, int bin, int state, int num_objects, double perc){
	sem_wait(lock);
	last_picked[bin] = state;
	last_picked_objects[bin] = num_objects;
	last_percent_full[bin] = perc;
	sem_post(lock);
}

void set_lightbar_picked(sem_t* lock, int bin, int value) {
	sem_wait(lock);
	last_picked_lightbar[bin] = value;
	sem_post(lock);
}

int bins_init(struct scale_list* l, int num_scales, int num_pseudo_scales) {
	
	l->scale = calloc((num_scales+num_pseudo_scales), sizeof(struct scale));
	l->num_scales = num_scales;
	l->num_pseudo_scales = num_pseudo_scales;

	last_picked = (int*)(malloc((sizeof(int))*num_scales));
	last_picked_lightbar = (int*)(malloc((sizeof(int))*num_scales));
	last_picked_objects = (int*)(malloc((sizeof(int))*num_scales));
	last_percent_full = (double*)(malloc((sizeof(double))*num_scales));

	int j;
	for(j=0; j<num_scales; j++) {
		last_picked[j] = -1;
		last_picked_lightbar[j] = -1;
		last_percent_full[j] = -1;
	}

	l->sem_weight = (sem_t*)(malloc((sizeof(sem_t))*num_scales));
	l->sem_lightbar = (sem_t*)(malloc((sizeof(sem_t))*num_scales));
	l->sem_restock = (sem_t*)(malloc((sizeof(sem_t))));
	sem_init(&(l->sem_restock[0]),1,1);

	int i;
	for(i=0; i < num_scales; i++) {
		
		l->scale[i].quantity_needed = 0;
		l->scale[i].percent_full = 100.00;
		l->scale[i].hand_in_bin = 0;
		l->scale[i].id  = i;
		l->scale[i].lightbar  = lightbars[i];
		l->scale[i].led = leds[i];
		l->scale[i].button = buttons[i];
		l->scale[i].sku  = skus[i];
		l->scale[i].use_given_weight = use_weights[i];
		l->scale[i].weight  = weights[i];
		l->scale[i].mode = modes[i];

		l->scale[i].lock_weight = &(l->sem_weight[i]);
		sem_init(&(l->sem_weight[i]),1,1);
		l->scale[i].lock_lightbar = &(l->sem_lightbar[i]);
		sem_init(&(l->sem_lightbar[i]),1,1);

		l->scale[i].lock_restock = &(l->sem_restock);

	}

	for(i=0; i<num_pseudo_scales; i++) {
		
		//Pseudo Scales use only a button and a LED to control picks

		l->scale[i+num_scales].quantity_needed = 1;
		l->scale[i+num_scales].percent_full = 100.0;
		l->scale[i+num_scales].id = i+num_scales;
		l->scale[i+num_scales].led = leds[i+num_scales];
		l->scale[i+num_scales].button = buttons[i+num_scales];
		l->scale[i+num_scales].sku = skus[i+num_scales];

		l->scale[i+num_scales].mode = 2;//Pseudo Scale, not actually a bin/scale
		
	}

	pthread_t pid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_create(&pid, &attr, restock_indicator, l);

//	read_configuration("/root/AKS/config.txt", l);

	return 1;
}

int scales_init(struct scale_list *s) {
	char *device_path = (char*) malloc(sizeof(char)*MAXBUFLEN);
	int i;
	for(i=0; i < s->num_scales; i++) {
		sprintf(device_path,"/dev/ttyUSB%d",scales[i]);
		s->scale[i].fid = fopen(device_path,"r");
		if(s->scale[i].fid == 1){
			//printf("Unable to open device %d\n",i);//ERROR!
			//break;
			return -200;
		}
	}
}

void *restock_indicator(void *arg) {
	struct scale_list *s = (struct scale_list*) arg;
	double minimum_weight;
	double temp_weight;

	int i;
	while(1) {
		usleep(1000000);
		minimum_weight = 1000000;
		for(i=0; i<s->num_scales; i++) {
			temp_weight = check_scale_percent_full(i, s->scale[i].lock_weight); 
			if(temp_weight < minimum_weight) {
				minimum_weight = temp_weight;
			}
		}
		if(minimum_weight < RED_THRESHOLD) {
			setLED(RED_RESTOCK_LED, 255);
			usleep(100);
			setLED(YELLOW_RESTOCK_LED, 0);
			usleep(100);
			setLED(GREEN_RESTOCK_LED, 0);
		} else if (minimum_weight < YELLOW_THRESHOLD) {
			setLED(RED_RESTOCK_LED, 0);
			usleep(100);
			setLED(YELLOW_RESTOCK_LED, 255);
			usleep(100);
			setLED(GREEN_RESTOCK_LED, 0);
		} else {
			setLED(RED_RESTOCK_LED, 0);
			usleep(100);
			setLED(YELLOW_RESTOCK_LED, 0);
			usleep(100);
			setLED(GREEN_RESTOCK_LED, 255);
		}
	}

}

void *picked(void *arg){
	struct scale *s = (struct scale*) arg; // Which scale is associated with this thread?

	//Basic scale information
	float max_weight = 45.0;
	float min_weight = -1.0;
	float min_object_weight = 0.02;
	float weight_range = max_weight - min_weight;

	//Info on specific objects to be stored in bin
	int given_object_weight_enabled = s->use_given_weight;
	float object_weight = s->weight;
	float object_weight_error = 0.0;

	//Weight buffer variables
	int weight_buffer_size = 35;
	float weight_buffer[weight_buffer_size];
	int wb_index = 0;
	int wb_noldest = 1;
	int wb_nnewest = weight_buffer_size;

	//Weight state variables
	int weight_state = 0;
	int old_weight_state = 0;

	//Object removal/addition detection variables
	int stable_cycle_count = 0;
	int observed_object_count = 0;
	float observed_object_weight = 0.0;
	float stable_weight = 0.0;

	//Full weight and percentage remaining variables
	int initial_weighing = 3*weight_buffer_size;
	float full_weight = 0.0;
	float bin_weight = W_BIN;
	float weight_percent_full = 100.0;
	
	//Updated every cycle variables
	float weight_change = 0.0;
	float avg_weight_change = 0.0;
	float previous_rolling_average = 0.0;
	float rolling_average = 0.0;
	float squared_std_dev = 0.0;
	float std_dev = 0.0;

	//Raw weight reading variables
	char buff[100];
	float weight = 0.0;

	//Clear weight buffer to 0
	int i;
	for(i = 0; i<weight_buffer_size; i++) {
		weight_buffer[i] = 0;
	}

	printf("THREAD BEGINNING PICKING PROCESS...\n");

	set_weight_picked(s->lock_weight, s->id, 0, 0, weight_percent_full);

	while(1) { 			// Main loop
		fgets(buff, 100, s->fid); 
		weight = atof(buff);
		if((weight == 0.0)||(isnan(weight))) {
			if(isnan(weight)) {
				printf("NANSSSSSSSSSSSS\n");
			}
			continue;//Bad data
		}
		//TEST
		//printf("TEST, RAW WEIGHT FROM BIN %d IS %f \n",s->id,weight);

		set_lightbar_picked(s->lock_lightbar, s->id, readLightBar(s->lightbar));	

		//Add weight to weight buffer
		wb_index++;
		if(wb_index>=weight_buffer_size) {
			wb_index = 0;
		} 

		wb_noldest++;
		if(wb_noldest>=weight_buffer_size) {
			wb_noldest = 0;
		}

		wb_nnewest++;
		if(wb_nnewest>=weight_buffer_size) {
			wb_nnewest = 0;
		}
	
		//Update the rolling average and standard deviation
		previous_rolling_average = rolling_average;

		//Remove the oldest weight change from the average, the change from the oldest value to the next oldest value
		avg_weight_change = avg_weight_change - (weight_buffer[wb_noldest] - weight_buffer[wb_index])/weight_buffer_size;
		//Remove the oldest weight value from the average
		rolling_average = rolling_average - weight_buffer[wb_index]/weight_buffer_size;

		weight_buffer[wb_index] = weight; 
		
		//Add the newest weight change to the average, the change from the previous newest value to the newest value
		avg_weight_change = avg_weight_change + (weight_buffer[wb_index] - weight_buffer[wb_nnewest])/weight_buffer_size;
		
		//Add the newest weight value to the average
		rolling_average = rolling_average + weight_buffer[wb_index]/weight_buffer_size;
	
		squared_std_dev = 0.0;
		int i;
		for(i = 0;i<weight_buffer_size;i++) {
			squared_std_dev += ((weight_buffer[i] - rolling_average)*(weight_buffer[i] - rolling_average))/weight_buffer_size;
		}

		std_dev = sqrtf(squared_std_dev);
		
		weight_change += rolling_average - previous_rolling_average;

		//TEST
		if(isnan(rolling_average)) {
			printf("NAN!!!!!!!!!!!!!! Bin %d, avg. value %f, new value %f, full weight %f\n", s->id, rolling_average, weight, full_weight);
		}
		//printf("Bin %d standard deviation is %f \n",s->id,std_dev);
		//printf("	Bin %d avg. weight is %f \n",s->id,rolling_average);
		//printf("	Bin %d avg. weight change is %f \n",s->id,avg_weight_change);

		//INITIALIZATION
		if(initial_weighing>0) {
			initial_weighing--;
			if(initial_weighing == 0) {
				full_weight = rolling_average;
				stable_weight = full_weight;
				//TEST
				printf("SHOULD HAPPEN ONCE FOR BIN %d, Full is %f \n",s->id, full_weight);
			}
			continue;	
		}

		//Check for state change and picks
		//State change/check
		weight_state;
		if(weight_change>min_object_weight) {
			weight_state = 1;//Increasing
			weight_change = 0;

			stable_cycle_count = 0;
			
			//TEST
			//printf("Bin %d Weight +++\n",s->id);

		} else if(weight_change<(-min_object_weight)) {
			weight_state = 2;//Decreasing
			weight_change = 0;

			stable_cycle_count = 0;

			//TEST
			//printf("Bin %d Weight ---\n",s->id);

		} else {
			weight_state = 0;//Stable/No Change

			stable_cycle_count++;
		}

		weight_percent_full = 100.0*((rolling_average-bin_weight)/(full_weight-bin_weight));

		//TEST
		//if(std_dev>(rolling_average/20.0)) {
		//printf("Bin %d is %f percent full\n",s->id,weight_percent_full);
		//}

		//Pick detection/checking
		object_weight_error = min_object_weight;
		if(given_object_weight_enabled) {
			if((object_weight/5.0)>min_object_weight) {
				object_weight_error = object_weight/5.0;
			} else {
				object_weight_error = OBJECT_WEIGHT_ACCEPTABLE_ERROR;
			}
		}

		if((weight_state == 0)&&(stable_cycle_count==STABLE_WAIT_CYCLES)) {//The weight must have stabilized

			//TEST
			//printf("Bin %d detected a previous change in weight\n",s->id);

			//Check to see if at least one object was taken
			if(given_object_weight_enabled) {
				if((stable_weight-rolling_average)>(object_weight_error)) {
					//Something was picked
					if((stable_weight-rolling_average)>(object_weight-object_weight_error)) {
						//At least one object was picked
						
						if((stable_weight-rolling_average)>((2*object_weight)-object_weight_error)) {
							//More than one object was picked
							
							int num_objs; 
							if((stable_weight-rolling_average)/(object_weight)<2) {
								num_objs = 2;//(stable_weight-rolling_average)/((2*object_weight)-object_weight_error);
							} else {
								num_objs = round((stable_weight-rolling_average)/(object_weight));
							}

							set_weight_picked(s->lock_weight, s->id, 2, num_objs, weight_percent_full);

							observed_object_count += 2;
							observed_object_weight += (((stable_weight-rolling_average)/2)-observed_object_weight)/observed_object_count;
							stable_weight = rolling_average;
						} else {
							set_weight_picked(s->lock_weight, s->id, 1, 1, weight_percent_full);

							observed_object_count++;
							observed_object_weight += ((stable_weight-rolling_average)-observed_object_weight)/observed_object_count;
							stable_weight = rolling_average;
						}
					} else { 
						set_weight_picked(s->lock_weight, s->id, 3, 0, weight_percent_full);

						stable_weight = rolling_average;
					}
				} else if((rolling_average-stable_weight)>(object_weight_error)) {
					int num_objs;
					num_objs = round((rolling_average-stable_weight)/(object_weight));
					set_weight_picked(s->lock_weight, s->id, 4, num_objs, weight_percent_full);

					stable_weight = rolling_average;
				}
			} else {
				if((stable_weight-rolling_average)>(object_weight_error)) {
					set_weight_picked(s->lock_weight, s->id, 1, 1, weight_percent_full);

					observed_object_count++;
					observed_object_weight += ((stable_weight-rolling_average)-observed_object_weight)/observed_object_count;
					//TEST
					printf("Observed object weight is %f for Bin %d \n",observed_object_weight,s->id);
					stable_weight = rolling_average;
				} else /*if(rolling_average>stable_weight)*/ {
					set_weight_picked(s->lock_weight, s->id, 0, 0, weight_percent_full);

					stable_weight = rolling_average;
				}
			}

			
		} else {
			//set_weight_picked(s->lock_weight, s->id, 0, 0, weight_percent_full);
		}

	}

}

