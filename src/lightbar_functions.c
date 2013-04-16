//By: Matt Smith
//Lightbar Functions


#include <stdio.h>
#include "i2c_functions.h"
#include "AKS_errors.h"


#define LIGHT_BAR_ENABLE_IC {0,0,0,0,0}
#define LIGHT_BAR_ENABLE_PIN {8,9,10,7,6}
#define LIGHT_BAR_SENSOR_IC {0,0,0,0,0}
#define LIGHT_BAR_SENSOR_PIN {11,12,13,4,3}

int default_lightbar_en_IC[] = LIGHT_BAR_ENABLE_IC;
int default_lightbar_en_Pin[] = LIGHT_BAR_ENABLE_PIN;
int default_lightbar_sensor_IC[] = LIGHT_BAR_SENSOR_IC;
int default_lightbar_sensor_Pin[] = LIGHT_BAR_SENSOR_PIN;

int* lightbar_en_IC = default_lightbar_en_IC;
int* lightbar_en_Pin = default_lightbar_en_Pin;
int* lightbar_sensor_IC = default_lightbar_sensor_IC;
int* lightbar_sensor_Pin = default_lightbar_sensor_Pin;

//Overrides default settings for light bar pins and IC:qs
void load_lightbar_setup(int* lb_en_ic, int* lb_en_pin, int* lb_sensor_ic, int* lb_sensor_pin) {
	lightbar_en_IC = lb_en_ic;
	lightbar_en_Pin = lb_en_pin;
	lightbar_sensor_IC = lb_sensor_ic;
	lightbar_sensor_Pin = lb_sensor_pin;
}

//Re-maps the numbering scheme for the lightbars, with the given array specifying which current lightbar number should be in the new array of lightbars
void remap_lightbar_numbers(int* newNumbers) {
	/*
	int* tempEnIC = (int*)malloc(sizeof(int)*sizeof(lightbar_en_IC));
	int* tempEnPin = (int*)malloc(sizeof(int)*sizeof(lightbar_en_Pin));
	int* tempSenIC = (int*)malloc(sizeof(int)*sizeof(lightbar_sensor_IC));
	int* tempSenPin = (int*)malloc(sizeof(int)*sizeof(lightbar_sensor_Pin));
	*/
	int tempEnIC[sizeof(lightbar_en_IC)];
	int tempEnPin[sizeof(lightbar_en_Pin)];
	int tempSenIC[sizeof(lightbar_sensor_IC)];
	int tempSenPin[sizeof(lightbar_sensor_Pin)];

	int i;

	//Note that all enable and sensor pin arrays should be the same length
	for(i=0;i<sizeof(lightbar_en_IC);i++) {
		tempEnIC[i] = lightbar_en_IC[i];
		tempEnPin[i] = lightbar_en_Pin[i];
		tempSenIC[i] = lightbar_sensor_IC[i];
		tempSenPin[i] = lightbar_sensor_Pin[i];
	}

	//Note that all enable and sensor pin arrays should be the same length
	for(i=0;i<sizeof(lightbar_en_IC);i++) {
		lightbar_en_IC[i] = tempEnIC[newNumbers[i]];
		lightbar_en_Pin[i] = tempEnPin[newNumbers[i]];
		lightbar_sensor_IC[i] = tempSenIC[newNumbers[i]];
		lightbar_sensor_Pin[i] = tempSenPin[newNumbers[i]];
	}

	/*
	free(tempEnIC);
	free(tempEnPin);
	free(tempSenIC);
	free(tempSenPin);
	*/
}

<<<<<<< HEAD
void enableLightBar(int lightbar) {
	i2c_setGPIOPin(lightbar_en_IC[lightbar],lightbar_en_Pin[lightbar],0);
	i2c_writeGPIOPins(lightbar_en_IC[lightbar]);
}

void disableLightBar(int lightbar) {
	i2c_setGPIOPin(lightbar_en_IC[lightbar],lightbar_en_Pin[lightbar],1);
	i2c_writeGPIOPins(lightbar_en_IC[lightbar]);
=======
int enableLightBar(int lightbar) {
	int err;

	err = i2c_setGPIOPin(lightbar_en_IC[lightbar],lightbar_en_Pin[lightbar],0);
	if(err<0) {
		return err;
	}

	err = i2c_writeGPIOPins(lightbar_en_IC[lightbar]);
	if(err<0) {
		return err;
	}

	return 0;
}

int disableLightBar(int lightbar) {
	int err;

	err = i2c_setGPIOPin(lightbar_en_IC[lightbar],lightbar_en_Pin[lightbar],1);
	if(err<0) {
		return err;
	}

	err = i2c_writeGPIOPins(lightbar_en_IC[lightbar]);
	if(err<0) {
		return err;
	}

	return 0;
>>>>>>> cd8a344e1562cbe025da487986df2a37154e67b3
}

int readLightBar(int lightbar) {
	int err;
	char tempBytes[2];
	err = i2c_readGPIOPins(lightbar_sensor_IC[lightbar], tempBytes);
	if(err<0) {
		return err;
	}
	return ((tempBytes[lightbar_sensor_Pin[lightbar]/8])>>(lightbar_sensor_Pin[lightbar]%8)&1);
}
