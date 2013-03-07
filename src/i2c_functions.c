//By: Matt Smith
//I2C Functions

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


#define DEFAULT_NUM_GPIO_IC 2
#define DEFAULT_GPIO_SLAVE_ADDRESSES {0x24,0x25}


#define GPIO_PIN_BYTES 2

int i2c_bus;

char i2c_GPIODevices[] = DEFAULT_GPIO_SLAVE_ADDRESSES;
char** i2c_GPIOPins;

//Sets up the GPIO expander ICs with the default settings
void setup_i2c_GPIO() {
	
	int i, err;

	if((i2c_bus = open("/dev/i2c-1",O_RDWR))<0) {
//		printf("Error opening I2C bus: %d\n",i2c_bus);
		perror("open error");
	}

//	if((err = ioctl(i2c_bus, I2C_SLAVE, 0x24))<0) {
//		printf("Error setting up I2C device: %d\n",err);
//		perror("ioctl error");
//		close(i2c_bus);
//		exit(-1);
//	}

	i2c_GPIOPins = malloc(DEFAULT_NUM_GPIO_IC*sizeof(char*));

	for(i = 0; i<DEFAULT_NUM_GPIO_IC; i++) {
		i2c_GPIOPins[i] = malloc(GPIO_PIN_BYTES);
		i2c_GPIOPins[i][0] = 255;	//DEPENDENT ON NUMBER OF GPIO PIN BYTES!
		i2c_GPIOPins[i][1] = 255;
	}
}

//Sets up the LED Driver ICs
void setup_i2c_LED_Driver() {

}

void set_i2c_slave_address(char slave_addr) {
	if(ioctl(i2c_bus, I2C_SLAVE, slave_addr)<0) {
		perror("Error setting slave address:");
	}
}

void i2c_select_GPIO_IC(int a) {
	set_i2c_slave_address(i2c_GPIODevices[a]);
}

//Sets the pin on the given IC to the given (0 or 1) value, DOES NOT WRITE THE VALUE TO THE IC
//ICs are ordered by their i2c addresses from lowest to highest and assigned numbers 0 - (number of ICs)
void i2c_setGPIOPin(int chip, int pin, char value) {
	//Bounds checks needed
	if(value==0) {
		i2c_GPIOPins[chip][pin/8] = (i2c_GPIOPins[chip][pin/8])&(~(1<<(pin%8)));
	} else {
		i2c_GPIOPins[chip][pin/8] = (i2c_GPIOPins[chip][pin/8])|(1<<(pin%8));
	}
}

//Sets all the pins of the given GPIO IC at once using a byte array. Writes the values to the IC.
void i2c_setGPIOPins(int chip, char* pins) {
	int i;
	for(i = 0; i<GPIO_PIN_BYTES; i++) {
		i2c_GPIOPins[chip][i] = pins[i];
	}
	i2c_writePins(chip);
}

//Reads all the pins on the given GPIO IC and stores the results in the given byte buffer.
void i2c_readGPIOPins(int chip, char* buf) {
	i2c_select_GPIO_IC(chip);
	if(read(i2c_bus, buf, GPIO_PIN_BYTES)!=GPIO_PIN_BYTES) {
		printf("Error reading bytes from I2C device\n");
	} else {//TESTING
		printf("Success reading bytes from I2C device\n");
	}
}

//Reads the value of the given pin on the given IC and returns a 0 for a low or a 1 for a high.
int i2c_readGPIOPin(int chip, int pin) {
	char temp[2];
	i2c_readGPIOPins(chip, temp);
	return (temp[pin/8]>>pin%8)&(1);
}

//Writes the data in the i2c_GPIOPins array to the GPIO IC
void i2c_writePins(int chip) {
	i2c_select_GPIO_IC(chip);
	if(write(i2c_bus, i2c_GPIOPins[chip], GPIO_PIN_BYTES)!=GPIO_PIN_BYTES) {
		printf("Error writing bytes to I2C device\n");
	} else {//TESTING
		printf("Success writing bytes to I2C device\n");
	}
}

void i2c_functions_dealloc() {
	int i;
	//free(i2c_GPIODevices);
	for(i = 0; i<sizeof(i2c_GPIOPins); i++) {
		free(i2c_GPIOPins[i]);
	}
	free(i2c_GPIOPins);
	close(i2c_bus);
}
