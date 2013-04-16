#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include "serial.h"

<<<<<<< HEAD
=======

extern const char* log_file_path;
extern const char* config_file_path;

>>>>>>> cd8a344e1562cbe025da487986df2a37154e67b3
// Writes bin configuration to file specified by path
int write_configuration(const char* path, struct scale_list* l);
int write_configuration_str(const char* path, char* buffer);

// Reads bin configuration to file specified by path
int read_configuration(const char* path, struct scale_list* l);
int read_configuration_str(const char* path, char* buffer);

// Logs message to file specified by path
int log_message(const char* path, char* message);

// Rotate logs... could be done with a cron job?
int rotate_logs();

<<<<<<< HEAD
//const char* log_file_path = "/root/AKS/log/log.txt";
//const char* config_file_path = "/root/AKS/config.txt";
=======

#else
const char* log_file_path = "/root/AKS/log/log.txt";
const char* config_file_path = "/root/AKS/config.txt";
>>>>>>> cd8a344e1562cbe025da487986df2a37154e67b3
#endif
