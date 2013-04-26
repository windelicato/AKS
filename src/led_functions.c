//By: Matt Smith

#include <stdio.h>
#include "i2c_functions.h"


#define DEBUG

#define DEFAULT_LED_IC {0,0,0,0,0,0,0,0,0};
#define DEFAULT_LED_PIN {4,3,2,1,0,5,7,8,9};

int default_led_IC[] = DEFAULT_LED_IC;
int default_led_Pin[] = DEFAULT_LED_PIN;

int* led_IC = default_led_IC;
int* led_Pin = default_led_Pin;

int led_number_of_leds = sizeof(default_led_IC)/sizeof(int);

//Initializes all LEDs to zero
int led_init() {
	
	#ifdef DEBUG
	//TEST
	printf("Running led_init()...\n");
	#endif

	int i, err;
	for(i = 0; i<(led_number_of_leds); i++) {
		
		//TEST
		printf("Turning off LED %d\n", i);

		if((err = i2c_setLEDDriverPin(led_IC[i],led_Pin[i], 0))<0) {
			//TEST
			printf("Error on turning off LED %d\n", i);
			return err;
		}
	}

	#ifdef DEBUG
	//TEST
	printf("Finished led_init().\n");
	#endif

	return 0;
}

void load_led_setup(int* newLEDICs, int* newLEDPins) {
	led_number_of_leds = sizeof(newLEDICs);
	led_IC = newLEDICs;
	led_Pin = newLEDPins;
}

void remap_led_numbers(int* newNumbers) {
	int tempLEDIC[sizeof(led_IC)];
	int tempLEDPin[sizeof(led_Pin)];

	int i;

	for(i=0;i<sizeof(led_IC);i++) {
		tempLEDIC[i] = led_IC[i];
		tempLEDPin[i] = led_Pin[i];
	}

	for(i=0;i<sizeof(led_IC);i++) {
		led_IC[i] = tempLEDIC[newNumbers[i]];
		led_Pin[i] = tempLEDPin[newNumbers[i]];
	}
}

int setLED(int led, char value) {
	int err;

	err = i2c_setLEDDriverPin(led_IC[led],led_Pin[led],value);
	if(err<0) {
		return err;
	}

	return 0;
}

//Shutdown by setting all LEDs to zero
int led_shutdown() {
	return led_init();
}
