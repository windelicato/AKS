#include <stdlib.h>
#include <stdio.h>
#include <mysql.h>
#include <string.h>

#include "scan.h"

#define MAXBUFLEN 512
#define MAX_ORDERS 1000

char * get_pick_packet() {
	char *message_send = malloc(sizeof(char) * 9 * MAXBUFLEN);
	char *order = malloc(sizeof(char) * MAXBUFLEN);
	memset(message_send, '\0', 9*MAXBUFLEN);
	memset(order, '\0', MAXBUFLEN);

	// Wait for new item to be scanned
	char * scan;
	do {
		scan = get_scan("/home/aks/scan.txt");
		usleep(500000);
	} while(strcmp(order,scan) == 0);
	strcpy(order, scan);

	// Get collateral from scan
	char * collat = get_collat(scan);

	// Create packet
	strcat(message_send, "1 ");
	strcat(message_send, scan);
	strcat(message_send, " ");
	strcat(message_send, collat);

	//printf("%s\n", message_send);

	return message_send;
}

char * get_scan(char* path) {
	char * scan = malloc(sizeof(char)*MAXBUFLEN);

	// Read in new scan
	FILE* file = fopen(path,"r");
	if (file == NULL){
		return NULL;
	}
	unsigned int size;

	do{
		fseek(file, 0L, SEEK_END);
		size = ftell(file);
		fseek(file, 0L, SEEK_SET);
	} while (size ==0);
	printf("%d\n", size);

	if(fread(scan, 1, size, file) != size ){
		printf("Could not read scan file");
	}

	file = fopen(path,"w");
	if (file == NULL){
		printf("Could not read scan file");
		return NULL;
	}
	fclose(file);

	return scan;
}

char * get_collat(char* STB_ID){

	char *returns = malloc(sizeof(char)*8*MAXBUFLEN);
	memset(returns,'\0',8*MAXBUFLEN);

	MYSQL *conn;
	MYSQL_RES * result;
	MYSQL_ROW row;
	char query[300] = { 0 };
	char base_query[300] = "SELECT c.sku_id FROM stb s left join collat c on s.sku_id = c.sku_id where s.stb_id = ";
	int num_fields;
	int i;
	int stb_id;

	conn = mysql_init(NULL);
	mysql_real_connect(conn, "localhost", "root","password", "olp", 0, NULL, 0);

	sprintf(query, "%s %s",base_query, STB_ID);

	mysql_query(conn, query );
	result = mysql_store_result(conn);

	num_fields = mysql_num_fields(result);
	if ( mysql_num_rows(result)> 0 ) {
		while (( row = mysql_fetch_row(result)))
		{
			for(i = 0; i < num_fields; i++)
			{
				strcat(returns,row[i] ? row[i] : "");
				strcat(returns," ");
			}
		}
	}
	return returns;

	mysql_free_result(result);
	mysql_close(conn);

	return 0;
}
