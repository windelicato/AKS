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
		float total = 0;
		int i;
		for(i=0; i<10; fgets(buff, 100, file)) {
			float weight;

			fgets(buff, 100, file);
			weight = atof(buff);
			if(weight != 0.0) {	
				weights[i] = weight;	
				total += weight;
				i++;
			}
		}
		printf("%f\n", total/10);
	}

	close(file);	

	return 0;
}
