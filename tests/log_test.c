#include<stdlib.h>
#include<stdio.h>

#include "configuration.h"

int main(int argc, const char *argv[])
{
	/* LOG TEST  */
	int n = 1;
	char * test_message = malloc(1000*sizeof(char));
	sprintf(test_message, "Picked item %d", n);

	log_message("/root/AKS/log/log.txt", test_message);
	

	/* READ TEST 

	*/

	struct scale_list* l = malloc(sizeof(struct scale_list));
	l->scale = calloc(20, sizeof(struct scale));

	read_configuration("/root/AKS/config.txt", l);
	int i;

	l->scale[9].id = 69;
	l->scale[9].lightbar = 101;
	l->scale[9].sku = 1010101;
	l->scale[9].weight = 9.233;
	l->size = 10;

	write_configuration("/root/AKS/config.txt", l);

	return 0;

}
