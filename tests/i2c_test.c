//By: Matt Smith
//I2C Test

#include "i2c_functions.h"
#include <stdlib.h>
#include <stdio.h>

#define BIG_SEC 8

int main() {
	
	char tempPins[2];
	tempPins[0] = 255;
	tempPins[1] = 255;
	
	char a;
	a=0;

	char tempArray[2];
	int i, sec;
	sec = 0;
	setup_i2c_GPIO();

	while(1) {
		
		if((a==0)&&(sec==BIG_SEC)) {
			a =1;
			tempPins[0] = 0x18;
			tempPins[1] = 0x38;
			i2c_setGPIOPins(0, tempPins);
		} else if(sec==BIG_SEC) {
			a = 0;
			tempPins[0] = 255;
			tempPins[1] = 255;
			i2c_setGPIOPins(0, tempPins);
		}

		//i2c_setGPIOPins(0, tempPins);
		
		i2c_readGPIOPins(0, tempArray);

		//for(i = 0; i<16; i++) {
		//	printf("Value on Pin %d is: %d \n", i, (tempArray[i/8]>>(i%8))&1);
		//}

		printf("Light Bar 1 Sensor: %d \n",(tempArray[11/8]>>(11%8))&1);
		printf("Light Bar 2 Sensor: %d \n",(tempArray[12/8]>>(12%8))&1);
		printf("Light Bar 3 Sensor: %d \n",(tempArray[13/8]>>(13%8))&1);
		printf("Light Bar 4 Sensor: %d \n",(tempArray[4/8]>>(4%8))&1);
		printf("Light Bar 5 Sensor: %d \n",(tempArray[3/8]>>(3%8))&1);
	
	if(sec >= BIG_SEC) {
		sec = 0;
	}
	sleep(1);
	sec++;
	}
	return 0;
}
