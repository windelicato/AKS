//By: Matt Smith
//Lightbar Functions


#include <stdio.h>
#include "i2c_functions.h"
#include "AKS_errors.h"

#define DEBUG

#define LIGHT_BAR_ENABLE_IC {0,0,0,0,0}
#define LIGHT_BAR_ENABLE_PIN {6,7,10,9,8}
#define LIGHT_BAR_SENSOR_IC {0,0,0,0,0}
#define LIGHT_BAR_SENSOR_PIN {3,4,13,12,11}

int lightbar_en_IC[] = LIGHT_BAR_ENABLE_IC;
int lightbar_en_Pin[] = LIGHT_BAR_ENABLE_PIN;
int lightbar_sensor_IC[] = LIGHT_BAR_SENSOR_IC;
int lightbar_sensor_Pin[] = LIGHT_BAR_SENSOR_PIN;

//Configure sensor pins as inputs
int lightbar_init() {

	#ifdef DEBUG
	//TEST
	printf("Running lightbar_init()...\n");
	#endif

	int i, err;
	//TEST
	printf("Lightbar sensor IC array is sizeof: %d\n", sizeof(lightbar_sensor_IC));

	for(i=0;i<(sizeof(lightbar_sensor_IC)/sizeof(int)); i++) {

		printf("LB init loop iter number %d\n",i);

		if((err = i2c_setGPIOPin(lightbar_sensor_IC[i],lightbar_sensor_Pin[i],1))<0) {
			//TEST
			printf("Error setting light bar sensor pin to input (high), light bar %d on pin %d. \n",i,lightbar_sensor_Pin[i]);
			return err;
		}
		if((err = i2c_writeGPIOPins(lightbar_sensor_IC[i]))<0) {
			printf("Error setting light bar sensor pin to input (high), light bar %d on pin %d. \n",i,lightbar_sensor_Pin[i]);
			return err;
		}
	}

	#ifdef DEBUG
	//TEST
	printf("Finished lightbar_init().\n");
	#endif

	return 0;
}

//Overrides default settings for light bar pins and IC:qs
void loadLightBarSetup(int* lb_en_ic, int* lb_en_pin, int* lb_sensor_ic, int* lb_sensor_pin) {
//	lightbar_en_IC = lb_en_ic;
//	lightbar_en_Pin = lb_en_pin;
//	lightbar_sensor_IC = lb_sensor_ic;
//	lightbar_sensor_Pin = lb_sensor_pin;
}

void enableLightBar(int lightbar) {
	i2c_setGPIOPin(lightbar_en_IC[lightbar],lightbar_en_Pin[lightbar],0);
	i2c_writeGPIOPins(lightbar_en_IC[lightbar]);
}

void disableLightBar(int lightbar) {
	i2c_setGPIOPin(lightbar_en_IC[lightbar],lightbar_en_Pin[lightbar],1);
	i2c_writeGPIOPins(lightbar_en_IC[lightbar]);
}

int readLightBar(int lightbar) {
	char tempBytes[2];
	i2c_readGPIOPins(lightbar_sensor_IC[lightbar], tempBytes);
	return ((tempBytes[lightbar_sensor_Pin[lightbar]/8])>>(lightbar_sensor_Pin[lightbar]%8)&1);
}

int lightbar_shutdown() {
	
	return 0;
}
