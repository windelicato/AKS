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
#include "AKS_errors.h"

<<<<<<< HEAD
=======
#define DEBUG

>>>>>>> cd8a344e1562cbe025da487986df2a37154e67b3
//GPIO IC Important Values
#define DEFAULT_NUM_GPIO_IC 2
#define DEFAULT_GPIO_SLAVE_ADDRESSES {0x24,0x25}
#define GPIO_PIN_BYTES 2

//LED IC Important Values
#define DEFAULT_NUM_LED_DRIVER_IC 1
<<<<<<< HEAD
#define DEFAULT_LED_DRIVER_SLAVE_ADDRESSES {0xC0}
=======
#define DEFAULT_LED_DRIVER_SLAVE_ADDRESSES {0x60}
>>>>>>> cd8a344e1562cbe025da487986df2a37154e67b3
#define LED_DRIVER_LED_PWM_BYTES 16
#define LED_DRIVER_IREFALL_REG 0x43
//The first of 16
#define LED_DRIVER_FIRST_LED_PWM_ADDRESS 0x0a
//OR this with the register address to enable the auto-increment of registers
#define LED_DRIVER_AUTO_INCREMENT 0x80
//I2C Initialization messages
//Initializes the 4 LEDOUT registers with the proper (b10101010) bits to enable individual control of LEDs
#define LED_DRIVER_LEDOUT_INIT_BYTES {0x82,0xAA,0xAA,0xAA,0xAA}
//Initializes the IREFALL register with a value for controlling the current of the LEDs
<<<<<<< HEAD
#define LED_DRIVER_IREFALL_INIT_BYTES {0x43,0xFF}
=======
#define LED_DRIVER_IREFALL_INIT_BYTES {0x43,0x4F}
>>>>>>> cd8a344e1562cbe025da487986df2a37154e67b3



#define I2C_COMMUNICATION_FAILURE_MAX 10
<<<<<<< HEAD
=======

const int gpio_ic_pin_bytes = GPIO_PIN_BYTES;
const int led_ic_pwm_bytes = LED_DRIVER_LED_PWM_BYTES;
>>>>>>> cd8a344e1562cbe025da487986df2a37154e67b3

int i2c_bus;

int i2c_gpio_ic_pin_bytes = GPIO_PIN_BYTES;

char i2c_GPIODevices[] = DEFAULT_GPIO_SLAVE_ADDRESSES;
char** i2c_GPIOPins;

char i2c_LEDDriverDevices[] = DEFAULT_LED_DRIVER_SLAVE_ADDRESSES;
char** i2c_LEDs;

//Every i2c communication error increments this counter, whenever the i2c communication failure max is reached, the counter is reset
//Other functions can manually reset the counter to enfore a time period requirement for the errors
int i2c_comm_fails;
<<<<<<< HEAD

//Whether or not there was a large number of i2c communication errors
int i2c_state_of_failure;

=======

//Whether or not there was a large number of i2c communication errors
int i2c_state_of_failure;

void failure() {
	i2c_comm_fails++;
	if(i2c_comm_fails>=I2C_COMMUNICATION_FAILURE_MAX) {
		aks_error(-304);
		i2c_state_of_failure = 1;
		i2c_comm_fails = 0;
	}
}

>>>>>>> cd8a344e1562cbe025da487986df2a37154e67b3
//Sets up the GPIO expander ICs with the default settings
int setup_i2c_GPIO() {
	
	int i;
<<<<<<< HEAD
=======

	#ifdef DEBUG
	//TEST
	printf("Running setup_i2c_GPIO() and allocating arrays for GPIO pins...\n");
	//TEST
	#endif
>>>>>>> cd8a344e1562cbe025da487986df2a37154e67b3

	i2c_GPIOPins = (char**)malloc(DEFAULT_NUM_GPIO_IC*sizeof(char*));

	for(i = 0; i<DEFAULT_NUM_GPIO_IC; i++) {
		i2c_GPIOPins[i] = (char*)malloc(GPIO_PIN_BYTES);
		i2c_GPIOPins[i][0] = 255;	//DEPENDENT ON NUMBER OF GPIO PIN BYTES!
		i2c_GPIOPins[i][1] = 255;
	}
	return 0;
}

//Sets up the LED Driver ICs
int setup_i2c_LED_Drivers() {
	
	int i,j,err;
	char ledout_init[] = LED_DRIVER_LEDOUT_INIT_BYTES;
	char irefall_init[] = LED_DRIVER_IREFALL_INIT_BYTES;
	
<<<<<<< HEAD
	//For each LED Driver IC, initialize its registers
	for(i = 0; i<DEFAULT_NUM_LED_DRIVER_IC; i++) {
		if((err = i2c_select_LED_Driver_IC(i))<0) {
			return err;
		}
		if(write(i2c_bus,ledout_init,sizeof(ledout_init))!=sizeof(ledout_init)) {
			aks_error(-303);
			failure();
			return -303;
		}
		if(write(i2c_bus,irefall_init,sizeof(irefall_init))!=sizeof(ledout_init)) {
			aks_error(-303);
			failure();
			return -303;
		}
	}

	i2c_LEDs = (char**)malloc(DEFAULT_NUM_LED_DRIVER_IC);

=======
	#ifdef DEBUG
	//TEST
	printf("Running setup_i2c_LED_Drivers()...\n");
	//TEST
	#endif

	//For each LED Driver IC, initialize its registers
	for(i = 0; i<DEFAULT_NUM_LED_DRIVER_IC; i++) {
		if((err = i2c_select_LED_Driver_IC(i))<0) {
			return err;
		}
		if(write(i2c_bus,ledout_init,sizeof(ledout_init))!=sizeof(ledout_init)) {
			aks_error(-303);
			failure();
			#ifdef DEBUG
			printf("Failed to write to LED Driver LEDOUT register\n");
			#endif
			return -303;
		}
		if(write(i2c_bus,irefall_init,sizeof(irefall_init))!=sizeof(irefall_init)) {
			aks_error(-303);
			failure();
			#ifdef DEBUG
			printf("Failed to write to LED Driver IREFALL register\n");
			#endif
			return -303;
		}
	}

	i2c_LEDs = (char**)malloc(DEFAULT_NUM_LED_DRIVER_IC*sizeof(char*));

	#ifdef DEBUG
	printf("Allocating arrays for LED Driver PWM values...\n");
	#endif

>>>>>>> cd8a344e1562cbe025da487986df2a37154e67b3
	for(i = 0; i<DEFAULT_NUM_LED_DRIVER_IC; i++) {
		
		i2c_LEDs[i] = (char*)malloc(LED_DRIVER_LED_PWM_BYTES);
		
		for(j = 0; j<LED_DRIVER_LED_PWM_BYTES; j++) {
			i2c_LEDs[i][j] = 255;
		}
	}
	return 0;
}

int i2c_init() {
<<<<<<< HEAD
=======
	int err;
>>>>>>> cd8a344e1562cbe025da487986df2a37154e67b3
	if((i2c_bus = open("/dev/i2c-1",O_RDWR))<0) {
		aks_error(-300);
		return -300;//Error opening i2c bus
	}
<<<<<<< HEAD
	setup_i2c_GPIO();
	setup_i2c_LED_Drivers();
=======
	if((err = setup_i2c_GPIO())<0) {
		return err;
	}
	if((err = setup_i2c_LED_Drivers())<0) {
		return err;
	}
>>>>>>> cd8a344e1562cbe025da487986df2a37154e67b3
	i2c_comm_fails = 0;
	i2c_state_of_failure = 0;
	return 0;
}

<<<<<<< HEAD
void failure() {
	i2c_comm_fails++;
	if(i2c_comm_fails>=I2C_COMMUNICATION_FAILURE_MAX) {
		aks_error(-304);
		i2c_state_of_failure = 1;//Other functions/threads will reset this variable at their descretion
		i2c_comm_fails = 0;
	}
}

=======
>>>>>>> cd8a344e1562cbe025da487986df2a37154e67b3
int set_i2c_slave_address(char slave_addr) {
	if(ioctl(i2c_bus, I2C_SLAVE, slave_addr)<0) {
		//Error changing slave address
		aks_error(-301);
		failure();
		return -301;
	}
	return 0;
}

int i2c_select_GPIO_IC(int a) {
	return set_i2c_slave_address(i2c_GPIODevices[a]);
}

int i2c_select_LED_Driver_IC(int a) {
	return set_i2c_slave_address(i2c_LEDDriverDevices[a]);
}

//Sets the pin on the given IC to the given (0 or 1) value, DOES NOT WRITE THE VALUE TO THE IC
//ICs are ordered by their i2c addresses from lowest to highest and assigned numbers 0 - (number of ICs)
int i2c_setGPIOPin(int chip, int pin, char value) {
	
	if(value==0) {
		i2c_GPIOPins[chip][pin/8] = (i2c_GPIOPins[chip][pin/8])&(~(1<<(pin%8)));
	} else {
		i2c_GPIOPins[chip][pin/8] = (i2c_GPIOPins[chip][pin/8])|(1<<(pin%8));
	}
	return 0;
}

int i2c_setLEDDriverPin(int chip, int pin, char value) {
	int err;
	char msg[2];

	msg[0] = (char)(LED_DRIVER_FIRST_LED_PWM_ADDRESS+pin);
	
	i2c_LEDs[chip][pin] = value;
	msg[1] = value;

	if((err = i2c_select_LED_Driver_IC(chip))<0) {
		return err;
	}
	if(write(i2c_bus,msg,sizeof(msg))!=sizeof(msg)) {
		aks_error(-303);
		failure();
		return -303;
	}

}

//Sets all the pins of the given GPIO IC at once using a byte array. Writes the values to the IC.
int i2c_setGPIOPins(int chip, char* pins) {
	int i, err;
	for(i = 0; i<GPIO_PIN_BYTES; i++) {
		i2c_GPIOPins[chip][i] = pins[i];
	}
	if((err = i2c_writeGPIOPins(chip))<0) {
		return err;
	}
	return 0;
}

int i2c_setLEDDriverPins(int chip, char* pins) {
	int i, err;
	char* msg = (char*)malloc(1+sizeof(pins));//pins should always contain 16 bytes

	msg[0] = (LED_DRIVER_AUTO_INCREMENT|LED_DRIVER_FIRST_LED_PWM_ADDRESS);

	for(i = 0; i<LED_DRIVER_LED_PWM_BYTES; i++) {
		i2c_LEDs[chip][i] = pins[i];
		msg[i+1] = pins[i];
	}
	if((err = i2c_select_LED_Driver_IC(chip))<0) {
		free(msg);
		return err;
	}
	if(write(i2c_bus,msg,sizeof(msg))!=sizeof(msg)) {
		free(msg);
		aks_error(-303);
		failure();
		return -303;
	}
	free(msg);
	return 0;
}

//Reads all the pins on the given GPIO IC and stores the results in the given byte buffer.
int i2c_readGPIOPins(int chip, char* buf) {
	int temp;
	if((temp = i2c_select_GPIO_IC(chip))<0) {
		return temp;
	}
	if(read(i2c_bus, buf, GPIO_PIN_BYTES)!=GPIO_PIN_BYTES) {
		//Error reading data from i2c device
		aks_error(-302);
		failure();
		return -302;
	} else {//TESTING
	//	printf("Success reading bytes from I2C device\n");
	}
	return 0;
}

int i2c_readLEDDriverPins(int chip, char* buf) {
	int temp;
	char start_read[1];

	start_read[0] = (LED_DRIVER_AUTO_INCREMENT|LED_DRIVER_FIRST_LED_PWM_ADDRESS);

	if((temp = i2c_select_LED_Driver_IC(chip))<0) {
		return temp;
	}
	if(write(i2c_bus,start_read,sizeof(start_read))!=sizeof(start_read)) {
		aks_error(-303);
		failure();
		return -303;
	}
	if(read(i2c_bus,buf, LED_DRIVER_LED_PWM_BYTES)!=LED_DRIVER_LED_PWM_BYTES) {
		aks_error(-302);
		failure();
		return -302;
	}
	return 0;
}

//Reads the value of the given pin on the given IC and returns a 0 for a low or a 1 for a high.
int i2c_readGPIOPin(int chip, int pin) {
	char temp[2];
	int err;
	if((err=i2c_readGPIOPins(chip, temp))<0) {
		return err;
	}
	return (temp[pin/8]>>pin%8)&(1);
}

int i2c_readLEDDriverPin(int chip, int pin) {
	int err, ret;
	char temp[1];
	temp[0] = (char)(LED_DRIVER_FIRST_LED_PWM_ADDRESS+pin);

	if((err=i2c_select_LED_Driver_IC(chip))<0) {
		return err;
	}
	if(write(i2c_bus,temp,sizeof(temp))!=sizeof(temp)) {
		aks_error(-303);
		failure();
		return -303;
	}
	if(read(i2c_bus,temp,1)!=1) {
		aks_error(-302);
		failure();
		return -302;
	}
	ret = temp[0];
	return ret;
}

//Writes the data in the i2c_GPIOPins array to the GPIO IC
int i2c_writeGPIOPins(int chip) {
	int temp;
	if((temp = i2c_select_GPIO_IC(chip))<0) {
		return temp;//Function already uses and returns AKS error codes
	}
	if(write(i2c_bus, i2c_GPIOPins[chip], GPIO_PIN_BYTES)!=GPIO_PIN_BYTES) {
		//Error writing data to i2c device
		aks_error(-303);
		failure();
		return -303;
	} else {//TESTING
		printf("Success writing bytes to I2C device\n");
	}
	return 0;
}

void i2c_functions_dealloc() {
	int i;
	
	for(i = 0; i<sizeof(i2c_GPIOPins); i++) {
		free(i2c_GPIOPins[i]);
	}
	for(i = 0; i<sizeof(i2c_LEDs); i++) {
		free(i2c_LEDs[i]);
	}

	free(i2c_GPIOPins);
	free(i2c_LEDs);
	close(i2c_bus);
}
