//By: Matt Smith

#include "AKS_errors.h"
#include "comm.h"
#include "configuration.h"
#include <stdio.h>

#define ERROR_LOG_MAX_BYTES 1000000
#define ERROR_LOG_PATH "Not setup yet"

const char* rpi_error_messages[] = {"Error Message 0", "Error Message 1", "Error Message 2"};
const char* network_error_messages[] = {"Error Message 100", "Error Message 101", "Error Message 102"};
const char* usb_error_messages[] = {"Error opening a USB device (200)", "Error Message 201", "Error Message 202"};
const char* i2c_error_messages[] = {"Error opening i2c bus (300)", "Error changing i2c slave address (301)", "Error reading data from i2c device (302)",
				"Error writing data to i2c device (303)"};
const char* other_error_messages[] = {"Error Message 400", "Error Message 401", "Error Message 402"};



//Initializes the AKS error reporting system
void aks_errors_init() {

}

//Sends an error message with information about the error to the server and updates the error log
void aks_error(int error_num) {
	//TEST
	//printf("AKS Error %d encountered\n", error_num);
	//return;
	//TEST
	
	if(error_num>0) {
		return;
	}

	if(error_num>-100) {
	//RPi Errors
	puts((rpi_error_messages[(-error_num)%100]));	
	log_message(log_file_path, rpi_error_messages[(-error_num)%100]);
	} else if(error_num>-200) {
	//Network Errors
	puts((network_error_messages[(-error_num)%100]));
	log_message(log_file_path, network_error_messages[(-error_num)%100]);
	} else if(error_num>-300) {
	//USB Errors
	puts((usb_error_messages[(-error_num)%100]));
	log_message(log_file_path, usb_error_messages[(-error_num)%100]);
	} else if(error_num>-400) {
	//I2C Errors
	puts((i2c_error_messages[(-error_num)%100]));
	log_message(log_file_path, i2c_error_messages[(-error_num)%100]);
	} else {
	//Other Errors
	puts((other_error_messages[(-error_num)%100]));
	log_message(log_file_path, i2c_error_messages[(-error_num)%100]);
	}

	//TEST
	//while(1) {
	//};
	//TEST
}

//Sends an error message, updates the error log, and shuts down the system
void aks_critical_error(int error_num) {
	aks_error(error_num);
	
	//TEST
	while(1);
	//TEST
	
	//Restart System
}
