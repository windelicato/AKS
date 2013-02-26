#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>

#define WEIGHTS 	5
#define AVGS 		30
#define DIFF 		0.01
#define W_POWER 	0.27
#define W_REMOTE 	0.40
#define W_PAMPHLET 	0.22
#define W_PAPER 	0.06
#define W_COMP 		0.50
#define ERROR 		0.01
#define STABLE_INTERVAL	25 

char* picked(FILE* file){
	float avg = 0;
	float W_MAX = 0;
	float W_EMPTY = 3.52;
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
		fgets(buff, 100, file);
		weight = atof(buff);
		if(weight != 0.0) {	// Read in valid weight

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
							printf("\n\nAdded Weight! (%f lb)\n\n", avg-prev_weight);
							W_MAX = avg;
							percent_full = (avg - W_EMPTY)*100 / (W_MAX-W_EMPTY);
						} else if( prev_weight - avg < ERROR) {
						} else if( prev_weight - avg < W_PAPER){
							percent_full = (avg - W_EMPTY)*100 / (W_MAX-W_EMPTY);
							printf("Paper picked (%d Percent Full)\n",(int)percent_full);
							return "1";
						} else if( prev_weight - avg < W_PAMPHLET){
							percent_full = (avg - W_EMPTY)*100/ (W_MAX-W_EMPTY);
							printf("Pamphlet picked (%d Percent Full)\n",(int)percent_full);
							return "2";
						} else if( prev_weight - avg < W_POWER){
							percent_full = (avg - W_EMPTY)*100 / (W_MAX-W_EMPTY);
							printf("Power cable picked (%d Percent Full)\n",(int)percent_full);
							return "3";
						} else if( prev_weight - avg < W_REMOTE){
							percent_full = (avg - W_EMPTY)*100 / (W_MAX-W_EMPTY);
							printf("Remote picked (%d Percent Full)\n",(int)percent_full);
							return "4";
						} else if( prev_weight - avg < W_COMP){
							percent_full = (avg - W_EMPTY)*100 / (W_MAX-W_EMPTY);
							printf("Comp cable  picked (%d Percent Full)\n",(int)percent_full);
							return "5";
						} else {
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

		}

	}

}

