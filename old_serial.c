#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include "lightbar_functions.h"
#include "i2c_functions.h"
#include "serial.h"

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

		l->scale[i].lock_weight = &(l->sem_weight[i]);
		sem_init(&(l->sem_weight[i]),1,1);
		l->scale[i].lock_lightbar = &(l->sem_lightbar[i]);
		sem_init(&(l->sem_lightbar[i]),1,1);

		disableLightBar(i);
	}

	return 1;
}

int scales_init(struct scale_list *s) {
	char *device_path = (char*) malloc(sizeof(char)*MAXBUFLEN);
	int i;
	for(i=0; i < s->size; i++) {
		sprintf(device_path,"/dev/ttyUSB%d",scales[i]);
		s->scale[i].fid = fopen(device_path,"r");
		s->scale[i].id  = i;
		s->scale[i].lightbar  = lightbars[i];
		s->scale[i].sku  = skus[i];
		if(s->scale[i].fid == 0){
			//printf("Unable to open device %d\n",i);//ERROR!
			//break;
			return -200;
		}
	}
}

void *picked(void *arg){
	struct scale *s = (struct scale*) arg; // Which scale is associated with this thread?

	float avg = 0;
	float W_MAX = 0;
	char buff[100];
	float weights[WEIGHTS];
	float avgs[AVGS];
	int cb_i = 0;
	int avg_i = 0;
	short stable = 1;
	float prev_weight = 0;


	printf("Reading...\n");

	while(1) { 			// Main loop
		float weight;
		fgets(buff, 100, s->fid); 
		weight = atof(buff);

		set_lightbar_picked(s->lock_lightbar, s->id, readLightBar(s->lightbar));	
		
		//TESTING
		int stable = 0;
		int weight_state = 0;//Stable/No Change: 0, Increasing: 1, Decreasing: 2
		float weight_percent_full = 100.0;
		float max_weight = 45.0;
		float min_weight = -1.0;
		float min_object_weight = 0.02;
		float object_weight;//Given by external source
		int obeserved_object_count = 0;
		float observed_object_weight;//The average weight of objects that we detect as removed objects
		float weight_range = max_weight - min_weight;

		float stable_weight = 0.0;
		int weight_buffer_size = 10;
		float weight_buffer[weight_buffer_size];
		int wb_index = 0;
		float weight_change, previous_rolling_average, rolling_average, squared_std_dev, std_dev;

		int rai;

		//Add weight to weight buffer

		if(wb_index>=weight_buffer_size) {
			wb_index = 0;
		} 
	
		//Update the rolling average and standard deviation

		rolling_average = rolling_average - weight_buffer[wb_index]/weight_buffer_size;
		squared_std_dev = squared_std_dev - (weight_buffer[wb_index]*weight_buffer[wb_index])/weight_buffer_size;

		weight_buffer[wb_index] = weight; 

		rolling_average = rolling_average + weight_buffer[wb_index]/weight_buffer_size;
		squared_std_dev = squared_std_dev + (weight_buffer[wb_index]*weight_buffer[wb_index])/weight_buffer_size;

		std_dev = sqrt(squared_std_dev);

		//Check for state change and picks

		//State change/check
		if(weight_change>min_object_weight) {
			weight_state = 1;//Increasing

		} else if(weight_change<(-min_object_weight)) {
			weight_state = 2;//Decreasing

		} else {
			weight_state = 0;//Stable/No Change
			
		}
		
		//Pick detection/checking
		object_weight_error = min_object_weight;
		if((weight_state = 0)&&(stable)) {//The weight must have stabilized

			//Check to see if at least one object was taken
			if(0) {
				if((stable_weight-rolling_average)>(object_weight-object_weight_error)) {
					//Need to account for multiple items being taken !!!
					observed_object_count++;
					observed_object_weight += (observed_object_weight-(stable_weight-rolling_average))/observed_object_count;
				}
			} else {
				if((stable_weight-rolling_average)>(observed_object_weight-object_weight_error)) {
					
					observed_object_count++;
					observed_object_weight += (observed_object_weight - (stable_weight-rolling_average))/observed_object_count;
				}
			}

			
			stable_weight = rolling_average;
		}

		//TESTING
			

	//OLD WEIGHT ALGORITHM BELOW		
	/*	if(weight != 0.0) {	// Read in valid weight

			weights[cb_i] = weight;	 // Add weight to circular buffer
			if(cb_i==WEIGHTS-1) {
				cb_i=0;
			} else {
				cb_i++;
			}
			int i;

			avg=0; 		// Find the average of all weights in the buffer
			for(i=0; i < WEIGHTS; i++) {
				avg += weights[i];
			}
			avg = avg/WEIGHTS;

			avgs[avg_i] = avg;	// Add this weight to the averages

			for(i = 0; i < AVGS ; i++){  // Wait for the weight to stabalize

				if ( i == 0 ) { 
					if ( fabs(avgs[AVGS-1] - avgs[i]) < DIFF) {
						stable = 1;
					} else{
						stable = 0;
						break;
					}
				}
				else {
					if ( fabs(avgs[i-1] - avgs[i]) < DIFF) {
						stable = 1;
					} else{
						stable = 0;
						break;
					}
				}
			}
			int t;
			if ( !stable ) { 	// Wait for STABLE_INTERVAL stable averages
				t=0;
			}
			else{
				if(t >= STABLE_INTERVAL) {
					if (t == STABLE_INTERVAL) {
						float percent_full = 0;
						if ( prev_weight - avg  < -ERROR ) {
							W_MAX = avg;
							percent_full = (avg - W_EMPTY)*100 / (W_MAX-W_EMPTY);
							set_weight_picked(s->lock_weight,s->id,percent_full);
						} else if( prev_weight - avg < ERROR) {
						} else {
							percent_full = (avg - W_EMPTY)*100 / (W_MAX-W_EMPTY);
							set_weight_picked(s->lock_weight,s->id,percent_full);
						}
					}
					prev_weight = avg;
					t++;
				}
				else {
					t++;
				}
			}

			if(avg_i==AVGS-1) { 	// Check circular buffer
				avg_i=0;
			} else {
				avg_i++;
			}
			
	*/	}

	}

}
