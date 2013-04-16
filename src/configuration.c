#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<time.h>

#include"serial.h"
#include"configuration.h"

char* config_file_path = "/root/AKS/config.txt";
char* log_file_path = "/root/AKS/log/log.txt";

char* format = "BIN:%d Lightbar:%d SKU:%d Weight:%lf\n";

int read_configuration_str(const char* path, char* buffer) {
	FILE* config = fopen(path, "r");

	fseek(config, 0L, SEEK_END);
	unsigned int size = ftell(config);
	fseek(config, 0L, SEEK_SET);

	if (fread(buffer, 1, size, config) != size) {
		printf("Could not read configuration file");
		log_message(log_file_path, "read_configuration(path,buffer) failed");
		fclose(config);
		return -1;
	}

	fclose(config);
	return 0;
}

int write_configuration_str(const char* path, char* buffer) {
	FILE* config = fopen(path, "w+");
	if(fwrite(buffer, 1, strlen(buffer), config) != strlen(buffer) ){
		printf("Could not write configuration file");
		log_message(log_file_path, "write_configuration_str(path,buffer) failed");
		fclose(config);
		return -1;
	}
	fclose(config);
	return 0;
}

// Writes bin configuration to file specified by path
int write_configuration(const char* path, struct scale_list* l) {
	FILE* config = fopen(path, "w+");
	int id, lightbar, sku;
	double weight;

	int i;
	for(i=0; i<l->size; i++) {
		fprintf(config, format, l->scale[i].id, l->scale[i].lightbar, l->scale[i].sku, l->scale[i].weight);
	}
	fclose(config);
	return 0; // Success
}

// Loads bin configuration to file specified by path
int read_configuration(const char* path, struct scale_list* l) {

	FILE* config = fopen(path, "r");
	int id, lightbar, sku;
	double weight;

	int num_read;
	int line_number = 0;
	while((num_read = fscanf(config, format, &id, &lightbar, &sku, &weight)) != EOF){
		if ( num_read != 4 ) {
			printf("Invalid configuration syntax at line %d\n", line_number);
			return -1;
		}
		l->scale[line_number].id = id;
		l->scale[line_number].lightbar = lightbar;
		l->scale[line_number].sku = sku;
		l->scale[line_number].weight = weight;
		line_number++;
	}
	
	fclose(config);
	return 0; // Success
}


// Logs message to logfile specified by path
int log_message(const char* path, char* message) {
	// Create timestamp
	time_t t;
	t = time(NULL);
	char *timestamp = asctime(localtime(&t));
	timestamp[strlen(timestamp)-1] = '\0'; 	//Remove trailing newline

	//Open file at descriptor
	FILE* log_file = fopen(path, "a");
	if(log_file == NULL) {
		// TO DO: UNABLE TO OPEN FILE ERROR
		perror("Unable to open file for appending: ");
		return -1;
	}

	fprintf(log_file, "[ %s ]\t%s\n", timestamp, message );

	fclose(log_file);

	return 0; // Success
}
