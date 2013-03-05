#include <stdlib.h>
#include <stdio.h>

#define MAX_BUF_LEN 100

char * get_scan(char* path) {
	FILE* file = fopen(path,"r");

	char * scan = (char*)malloc(sizeof(char)*MAX_BUF_LEN);

	do {
		*scan++ = (char) fgetc(file);
	} while (*scan != '\n');

	return scan
}
