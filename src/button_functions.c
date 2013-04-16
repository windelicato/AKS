//By: Matt Smith

#include <stdio.h>
#include <stdlib.h>
#include "i2c_functions.h"

#define DEFAULT_BUTTON_IC {1,1,1,1,1}
#define DEFAULT_BUTTON_PIN {0,1,2,3,4}
#define GPIO_PIN_BYTES i2c_gpio_ic_pin_bytes

extern int i2c_gpio_ic_pin_bytes;

int default_button_IC[] = DEFAULT_BUTTON_IC;
int default_button_Pin[] = DEFAULT_BUTTON_PIN;

int* button_IC = default_button_IC;
int* button_Pin = default_button_Pin;

void load_button_setup(int* buttonICs, int* buttonPins) {
	button_IC = buttonICs;
	button_Pin = buttonPins;
}

void remap_button_numbers(int* newNumbers) {
	int tempbIC[sizeof(button_IC)];
	int tempbPin[sizeof(button_Pin)];

	int i;

	for(i=0;i<sizeof(button_IC);i++) {
		tempbIC[i] = button_IC[i];
		tempbPin[i] = button_Pin[i];
	}

	for(i=0;i<sizeof(button_IC);i++) {
		button_IC[i] = tempbIC[newNumbers[i]];
		button_Pin[i] = tempbPin[newNumbers[i]];
	}
}

int getButtonState(int button) {
	return i2c_readGPIOPin(button_IC[button],button_Pin[button]);
}

int getAllButtonsState(int* buf) {
	int temp, i, j, err;
	int tempICArray[sizeof(button_IC)];
	int* tempArray;

	tempICArray[0] = button_IC[0];
	j = 0;
	for(i = 0;i<sizeof(button_IC);i++) {
		if(button_IC[i]!=tempICArray[j]) {
			j++;
			tempICArray[j] = button_IC[i];
		}
	}

	tempArray = (char*)malloc((j+1)*GPIO_PIN_BYTES);//NEED TO GET NUM OF GPIO PIN BYTES

	for(i=0;i<=j;i++) {
		err = i2c_readGPIOPins(tempICArray[i],(char*)(tempArray+(i*GPIO_PIN_BYTES)));
		if(err<0) {
			return err;
		}
	}
	
	for(i=0;i<sizeof(button_IC);i++) {
		buf[i] = ((tempArray[(button_IC[i]*GPIO_PIN_BYTES)+(button_Pin[i]/8)])>>(button_Pin[i]%8))%2;//8 is the number of bits in a byte
	}
	
	free(tempArray);

	return 0;
}
