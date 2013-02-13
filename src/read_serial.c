#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>

int main(int argc, const char *argv[])
{

	FILE *file = fopen("/dev/ttyUSB0","r");
	if(!file){
		perror("Unable to open device");
		exit(-1);
	}

	printf("Reading...\n");

	while(1){
		char buff[100];
		float weights[10];
		int i;
		for(i=0; i<10; fgets(buff, 100, file)) {
			float weight;
			fgets(buff, 100, file);
			weight = atof(buff);
			if(weight != 0.0) {	
				weights[i] = weight;	
				if(i==9) {
					i=0;
				}
				else {
					i++;
				}

				int j;
				float avg = 0;
				for(j=0; j < 10; j++) {
					avg += weights[j];
				}
				printf("%f\n", avg/10);
			}
		}
	}

	close(file);	

	return 0;
}
