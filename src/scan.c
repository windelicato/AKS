#include <stdlib.h>
#include <stdio.h>
#include <mysql.h>
#include <string.h>

#define MAX_BUF_LEN 100
#define MAX_ORDERS 1000

char * get_scan(char* path) {
	FILE* file = fopen(path,"r");
	if (file == NULL){
		return NULL;
	}

	char * scan = malloc(sizeof(char)*MAX_BUF_LEN);
	int c,n =0;

	while ((c = fgetc(file)) != '\n'){
		scan[n++] = (char) c;
	}
	scan[n] = '\0';

//	printf("%s\n", scan);kkk

	return scan;
}

char * get_collat(char* STB_ID){

	char *returns = malloc(sizeof(char)*8*MAX_BUF_LEN);
	memset(returns,'\0',8*MAX_BUF_LEN);

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
