//By: Matt Smith
//I2C Test

#include "i2c_functions.h"
#include <stdlib.h>
#include <stdio.h>

int main() {
	
	char tempPins[2];
	tempPins[0] = 255;
	tempPins[1] = 255;
	
	char a;

	a=0;
	setup_i2c_GPIO();

	while(1) {

		i2c_setGPIOPins(0, tempPins);
		//i2c_setGPIOPin(0,0,a);
		if(tempPins[0] == 255) {
			tempPins[0] = 0;
			tempPins[1] = 0;
			a=0;
			printf("Set pins to 1\n");	
		} else {
			tempPins[0] = 255;
			tempPins[1] = 255;
			a=1;
			printf("Set pins to 0\n");
		}
	sleep(5);

	}
	return 0;
}
