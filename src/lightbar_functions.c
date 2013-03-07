//By: Matt Smith
//Lightbar Functions


#include <stdio.h>
#include "i2c_functions.h"

#define LIGHT_BAR_ENABLE_IC {0,0,0,0,0}
#define LIGHT_BAR_ENABLE_PIN {8,9,10,7,6}
#define LIGHT_BAR_SENSOR_IC {0,0,0,0,0}
#define LIGHT_BAR_SENSOR_PIN {11,12,13,4,3}

int lightbar_en_IC[] = LIGHT_BAR_ENABLE_IC;
int lightbar_en_Pin[] = LIGHT_BAR_ENABLE_PIN;
int lightbar_sensor_IC[] = LIGHT_BAR_SENSOR_IC;
int lightbar_sensor_Pin[] = LIGHT_BAR_SENSOR_PIN;

//Overrides default settings for light bar pins and IC:qs
void loadLightBarSetup(int* lb_en_ic, int* lb_en_pin, int* lb_sensor_ic, int* lb_sensor_pin) {
//	lightbar_en_IC = lb_en_ic;
//	lightbar_en_Pin = lb_en_pin;
//	lightbar_sensor_IC = lb_sensor_ic;
//	lightbar_sensor_Pin = lb_sensor_pin;
}

void enableLightBar(int lightbar) {
	i2c_setGPIOPin(lightbar_en_IC[lightbar],lightbar_en_Pin[lightbar],0);
	i2c_writePins(lightbar_en_IC[lightbar]);
}

void disableLightBar(int lightbar) {
	i2c_setGPIOPin(lightbar_en_IC[lightbar],lightbar_en_Pin[lightbar],1);
	i2c_writePins(lightbar_en_IC[lightbar]);
}

int readLightBar(int lightbar) {
	char tempBytes[2];
	i2c_readGPIOPins(lightbar_sensor_IC[lightbar], tempBytes);
	return ((tempBytes[lightbar_sensor_Pin[lightbar]/8])>>(lightbar_sensor_Pin[lightbar]%8)&1);
}
