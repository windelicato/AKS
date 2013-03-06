//By: Matt Smith
//I2C Functions

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "i2c_functions.h"


#define GPIO_PIN_BYTES 2

int* i2c_GPIODevices;
char** i2c_GPIOPins;

//Sets up the GPIO expander ICs
void setup_i2c_GPIO() {
	
	int addr,err;
	addr = 0x24;

	//TEMPORARY
	i2c_GPIODevices = malloc(sizeof(int));
	if((i2c_GPIODevices[0] = open("/dev/i2c-1",O_RDWR))<0) {
//		printf("Error opening I2C bus: %d\n",i2c_GPIODevices[0]);
		perror("open error");
	}

	if((err = ioctl(i2c_GPIODevices[0], I2C_SLAVE, 0x24))<0) {
//		printf("Error setting up I2C device: %d\n",err);
		perror("ioctl error");
		close(i2c_GPIODevices[0]);
		exit(-1);
	}
	i2c_GPIOPins = malloc(sizeof(char*));
	i2c_GPIOPins[0] = malloc(GPIO_PIN_BYTES);

	i2c_GPIOPins[0][0] = 255;
	i2c_GPIOPins[0][1] = 255;
	//END OF TEMPORARY
}

//void setup_i2c_GPIO(){
//	wiringPiSetup();
//	
//	i2c_GPIOPins = malloc(sizeof(char*));
//	i2c_GPIOPins[0] = malloc(GPIO_PIN_BYTES);
//	i2c_GPIODevices = malloc(sizeof(int));
//
//	i2c_GPIOPins[0][0] = 255;
//	i2c_GPIOPins[0][1] = 255;
//	i2c_GPIODevices[0] = wiringPiI2CSetup(0x24);
//}


//Sets up the LED Driver ICs
void setup_i2c_LED_Driver() {

}

//Sets the pin on the given IC to the given (0 or 1) value
//ICs are ordered by their i2c addresses from lowest to highest and assigned numbers 0 - (number of ICs)
void i2c_setGPIOPin(int chip, int pin, char value) {
	//Bounds checks needed

	i2c_GPIOPins[chip][pin/8] = (i2c_GPIOPins[chip][pin/8])|(value<<(pin-(8*(pin/8))));
	i2c_writePins(chip);
}

void i2c_setGPIOPins(int chip, char* pins) {
	int i;
	for(i = 0; i<GPIO_PIN_BYTES; i++) {
		i2c_GPIOPins[chip][i] = pins[i];
	}
	i2c_writePins(chip);
}

//Reads the pin on the given IC and returns a 0 for a low and a 1 for a high
//ICs are ordered by their i2c addresses from lowest to highest and assigned numbers 0 - (number of ICs)
int i2c_readGPIOPin(int chip, int pin) {

	return 0;
}

void i2c_writePins(int chip) {
	if(write(i2c_GPIODevices[chip], i2c_GPIOPins[chip], GPIO_PIN_BYTES)!=GPIO_PIN_BYTES) {
		printf("Error writing bytes to I2C device\n");
	} else {
		printf("Success writing bytes to I2C device\n");
	}
}

