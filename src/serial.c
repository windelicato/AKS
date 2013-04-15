#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include "lightbar_functions.h"
#include "i2c_functions.h"
#include "serial.h"
#include "AKS_errors.h"

#define WEIGHTS 	5
#define AVGS 		30
#define MAXBUFLEN 100
#define DIFF 		0.01
#define W_POWER 	0.27
#define W_REMOTE 	0.40
#define W_PAMPHLET 	0.22
#define W_PAPER 	0.06
#define W_COMP 		0.50
#define ERROR 		0.01
#define STABLE_INTERVAL	25 
#define W_EMPTY  	0.05

int lightbars[] = {2, 0, 1, 4, 3};
int scales[] = {2, 0, 3, 4, 1};
int skus[] = {11000000, 11018971, 11073767, 11083039, 11180134};

int* 	last_picked_lightbar;//Array of ints for each bin, like last_picked
int*    last_picked;//Array of ints for each bin, bins set value to their bin # when picked
double* last_percent_full;//Array of percent fulls for each bin

int check_weight_picked(int bin, sem_t* lock){ 
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

double check_percent_full(int bin, sem_t* lock){ 
	int temp;
	sem_wait(lock);
	temp = last_percent_full[bin];
	//last_percent_full[bin] = -1;
	sem_post(lock);
	return temp;
}

void set_weight_picked(sem_t* lock, int bin, double perc){
	sem_wait(lock);
	last_picked[bin] = bin;
	last_percent_full[bin] = perc;
	sem_post(lock);
}

void set_lightbar_picked(sem_t* lock, int bin, int value) {
	sem_wait(lock);
	last_picked_lightbar[bin] = value;
	sem_post(lock);
}

int bins_init(struct scale_list* l, int num_scales) {
	
	l->scale = calloc(num_scales, sizeof(struct scale));
	l->size = num_scales;

	last_picked = (int*)(malloc((sizeof(int))*num_scales));
	last_picked_lightbar = (int*)(malloc((sizeof(int))*num_scales));
	last_percent_full = (double*)(malloc((sizeof(double))*num_scales));

	int j;
	for(j=0; j<num_scales; j++) {
		last_picked[j] = -1;
		last_picked_lightbar[j] = -1;
		last_percent_full[j] = -1;
	}

	l->sem_weight = (sem_t*)(malloc((sizeof(sem_t))*num_scales));
	l->sem_lightbar = (sem_t*)(malloc((sizeof(sem_t))*num_scales));

	int i;
	for(i=0; i < num_scales; i++) {
		
		
		l->scale[i].quantity_needed = 0;
		l->scale[i].percent_full = 1.00;
		l->scale[i].hand_in_bin = 0;
		l->scale[i].id  = i;
		l->scale[i].lightbar  = lightbars[i];
		l->scale[i].sku  = skus[i];

		l->scale[i].lock_weight = &(l->sem_weight[i]);
		sem_init(&(l->sem_weight[i]),1,1);
		l->scale[i].lock_lightbar = &(l->sem_lightbar[i]);
		sem_init(&(l->sem_lightbar[i]),1,1);

		disableLightBar(i);
	}

	read_configuration("/root/AKS/config.txt", l);

	return 1;
}

int scales_init(struct scale_list *s) {
	char *device_path = (char*) malloc(sizeof(char)*MAXBUFLEN);
	int i;
	for(i=0; i < s->size; i++) {
		sprintf(device_path,"/dev/ttyUSB%d",scales[i]);
		s->scale[i].fid = fopen(device_path,"r");
		if(s->scale[i].fid == 1){
			//printf("Unable to open device %d\n",i);//ERROR!
			//break;
			return -200;
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
	int given_object_weight_enabled = 0;
	float object_weight = 0.02;
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
	memset(weight_buffer,0,weight_buffer_size);

	set_weight_picked(s->lock_weight, s->id, weight_percent_full);

	while(1) { 			// Main loop
		fgets(buff, 100, s->fid); 
		weight = atof(buff);
		if(weight == 0.0) {
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

		weight_percent_full = 100.0*(rolling_average/full_weight);

		//TEST
		//if(std_dev>(rolling_average/20.0)) {
		//printf("Bin %d is %f percent full\n",s->id,weight_percent_full);
		//}

		//Pick detection/checking
		object_weight_error = min_object_weight;
		if((weight_state == 0)&&(stable_cycle_count==10)) {//The weight must have stabilized

			//TEST
			//printf("Bin %d detected a previous change in weight\n",s->id);

			//Check to see if at least one object was taken
			if(given_object_weight_enabled) {
				if((stable_weight-rolling_average)>(object_weight_error)) {
					//Need to account for multiple items being taken !!!
					set_weight_picked(s->lock_weight, s->id, weight_percent_full);

					observed_object_count++;
					observed_object_weight += ((stable_weight-rolling_average)-observed_object_weight)/observed_object_count;
					stable_weight = rolling_average;
				} else if(rolling_average>stable_weight) {
					stable_weight = rolling_average;
				}
			} else {
				if((stable_weight-rolling_average)>(object_weight_error)) {
					
					observed_object_count++;
					observed_object_weight += ((stable_weight-rolling_average)-observed_object_weight)/observed_object_count;
					//TEST
					printf("Observed object weight is %f for Bin %d \n",observed_object_weight,s->id);
					stable_weight = rolling_average;
				} else if(rolling_average>stable_weight) {
					stable_weight = rolling_average;
				}
			}

			
		}

	}

}

