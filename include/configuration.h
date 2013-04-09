#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include "serial.h"

// Writes bin configuration to file specified by path
int write_configuration(char* path, struct scale_list* l);

// Reads bin configuration to file specified by path
int read_configuration(char* path, struct scale_list* l);

// Logs message to file specified by path
int log_message(char* path, char* message);

// Rotate logs... could be done with a cron job?
int rotate_logs();

#endif
