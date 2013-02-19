#include<stdio.h>
#include<stdlib.h>
#include<mysql.h>
#include<string.h>


#define MAX_ORDERS 1000

int main(int argc, char **argv) {

//	printf("Client Version: %s\n", mysql_get_client_info());
	
	MYSQL *conn;
	MYSQL_RES * result;
	MYSQL_ROW row;
	char query[300] = { 0 };
	char base_query[300] = "SELECT c.sku_id, c.sku_disp FROM stb s left join collat c on s.sku_id = c.sku_id where s.stb_id = ";
	int num_fields;
	int i;
	int stb_id;

	conn = mysql_init(NULL);
	mysql_real_connect(conn, "localhost", "root","password", "olp", 0, NULL, 0);

	for(stb_id = 0; stb_id < MAX_ORDERS; stb_id++){
		sprintf(query, "%s %d",base_query, stb_id);

		mysql_query(conn, query );
		result = mysql_store_result(conn);

		num_fields = mysql_num_fields(result);
		if ( mysql_num_rows(result)> 0 ) {
			printf("\n\nITEMS FOR  ORDER # %d \n", stb_id);
			printf("---------------------------\n");

			while (( row = mysql_fetch_row(result)))
			{
				for(i = 0; i < num_fields; i++)
				{
					printf("%s\t", row[i] ? row[i] : "NULL");
				}
				printf("\n");
			}
		}
	}

	mysql_free_result(result);
	mysql_close(conn);


	return 0;
}
