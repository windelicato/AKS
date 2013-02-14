#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>
#include<math.h>

#define WEIGHTS 5
#define AVGS 20
#define DIFF  		0.01
#define W_POWER 	0.27
#define W_REMOTE 	0.40
#define W_PAMPHLET 	0.22
#define W_PAPER	 	0.06
#define ERROR		0.01
#define STABLE_INTERVAL 25

float avg = 0;
float W_MAX = 0;
float W_EMPTY = 3.52;

int main(int argc, const char *argv[])
{
	char buff[100];
	float weights[WEIGHTS];
	float avgs[AVGS];
	int cb_i = 0;
	int avg_i = 0;
	short stable = 1;
	float prev_weight = 0;

	FILE *file = fopen("/dev/ttyUSB0","r");
	if(!file){
		perror("Unable to open device");
		exit(-1);
	}

	printf("Reading...\n");

	char input[20];
	printf("Calibrate? Y/N\n");
	fgets(input,sizeof input, stdin);
	if (input[0] == 'Y') {
		printf("Empty the bin. Press enter to set weight\n");
		fgets(input, sizeof input, stdin);
		fgets(buff, 100, file);
		float weight = atof(buff);
		while (weight != 0) {
			fgets(buff, 100, file);
			weight = atof(buff);
		}
		W_EMPTY = weight;
		printf("Add items to bin\n");
		fgets(input, sizeof input, stdin);
		fgets(buff, 100, file);
		weight = atof(buff);
		while (weight != 0) {
			fgets(buff, 100, file);
			weight = atof(buff);
		}
		W_MAX = weight;
	}


	while(fgets(buff, 100, file)) {
		float weight;
		fgets(buff, 100, file);
		weight = atof(buff);
		if(weight != 0.0) {	
			weights[cb_i] = weight;	
			if(cb_i==WEIGHTS-1) {
				cb_i=0;
			} else {
				cb_i++;
			}
			int i;

			avg=0;
			for(i=0; i < WEIGHTS; i++) {
				avg += weights[i];
			}
			avg = avg/WEIGHTS;
			avgs[avg_i] = avg;	

			for(i = 0; i < AVGS ; i++){

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
			
	//		float stable_avg=0;
	//		for(i = 0; i < AVGS ; i++){
	//			stable_avg+=avgs[i];
	//		}
	//		stable_avg/= AVGS;
	//		if( fabs(stable_avg - avg) < DIFF) {
	//			stable = 1;
	//		} else {
	//			stable = 0;
	//		}


			int t;
			if ( !stable ) {
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
							printf("Paper picked (%d Percent Full)\n",(int)percent_full);
						} else if( prev_weight - avg < ERROR) {
						} else if( prev_weight - avg < W_PAPER){
							percent_full = (avg - W_EMPTY)*100 / (W_MAX-W_EMPTY);
							printf("Paper picked (%d Percent Full)\n",(int)percent_full);
						} else if( prev_weight - avg < W_PAMPHLET){
							percent_full = (avg - W_EMPTY)*100/ (W_MAX-W_EMPTY);
							printf("Pamphlet picked (%d Percent Full)\n",(int)percent_full);
						} else if( prev_weight - avg < W_POWER){
							percent_full = (avg - W_EMPTY)*100 / (W_MAX-W_EMPTY);
							printf("Power cable picked (%d Percent Full)\n",(int)percent_full);
						} else if( prev_weight - avg < W_REMOTE){
							percent_full = (avg - W_EMPTY)*100 / (W_MAX-W_EMPTY);
							printf("Remote picked (%d Percent Full)\n",(int)percent_full);
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


			if(avg_i==AVGS-1) {
				avg_i=0;
			} else {
				avg_i++;
			}

		}


	}

	close(file);	

	return 0;
}
