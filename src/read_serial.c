#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>
#include<math.h>

#define WEIGHTS 5
#define AVGS 20
#define DIFF 0.01

float avg = 0;

int main(int argc, const char *argv[])
{

	FILE *file = fopen("/dev/ttyUSB0","r");
	if(!file){
		perror("Unable to open device");
		exit(-1);
	}

	printf("Reading...\n");

	char buff[100];
	float weights[WEIGHTS];
	float avgs[AVGS];
	int cb_i = 0;
	int avg_i = 0;
	short stable = 1;

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
		}

		int i;

		avg=0;
		for(i=0; i < WEIGHTS; i++) {
			avg += weights[i];
		}
		avg = avg/WEIGHTS;

		avgs[avg_i] = avg;	

		if ( avg_i == 0 ) {
			if ( fabs(avgs[9] - avgs[avg_i]) < DIFF) {
				stable = 1;
			} else{
				stable = 0;
			}
		}
		else {
			if ( fabs(avgs[avg_i-1] - avgs[avg_i]) < DIFF) {
				stable = 1;
			} else{
				stable = 0;
			}
		}

		if ( !stable ) {
			printf("unstable!\n");
		}
		else{
			printf("\n");
		}

		if(avg_i==AVGS-1) {
			avg_i=0;
		} else {
			avg_i++;
		}


	}

	close(file);	

	return 0;
}
