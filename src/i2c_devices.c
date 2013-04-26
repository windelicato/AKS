//By: Matt Smith

#include <stdio.h>

#include "i2c_functions.h"
#include "lightbar_functions.h"
#include "button_functions.h"
#include "led_functions.h"

#include "i2c_devices.h"

#define DEBUG

int i2c_devices_init() {

	#ifdef DEBUG	
	//TEST
	printf("Running i2c_devices_init()...\n");
	#endif

	int err;
	
	if((err = i2c_init())<0) {
		return err;
	}

	if((err = lightbar_init())<0) {
		return err;
	}
	if((err = button_init())<0) {
		return err;
	}
	if((err = led_init())<0) {
		return err;
	}
	
	#ifdef DEBUG
	//TEST
	printf("Finished i2c_devices_init().\n");
	#endif

	return 0;
}

int i2c_devices_shutdown() {
	int err;

	if((err = lightbar_shutdown())<0) {
		return err;
	}
	if((err = button_shutdown())<0) {
		return err;
	}
	if((err = led_shutdown())<0) {
		return err;
	}

	if((err = i2c_shutdown()) <0) {
		return err;
	}

	return 0;
}
