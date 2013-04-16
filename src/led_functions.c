//By: Matt Smith

#include "i2c_functions.h"

#define DEFAULT_LED_IC {0,0,0,0,0};
#define DEFAULT_LED_PIN {0,1,2,3,4};

int default_led_IC[] = DEFAULT_LED_IC;
int default_led_Pin[] = DEFAULT_LED_PIN;

int* led_IC = default_led_IC;
int* led_Pin = default_led_Pin;

void load_led_setup(int* newLEDICs, int* newLEDPins) {
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
