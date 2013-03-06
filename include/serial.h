#ifndef _SERIAL_H_
#define _SERIAL_H_
#include<semaphore.h>

// Information pretaining to each individual scale
struct scale{
	FILE* fid;
	int id;
	int sku;
	double weight;
	sem_t *lock;
};

// List of scales with controlling semaphore
struct scale_list{
	int size;
	sem_t sem;
	struct scale *scale;
};

// Open scale devices for reading / riding
int open_scales(struct scale_list *scales);

// Check the most recently picked item
int check_picked(sem_t* lock);

// Check the most recently picked bin percent full
double check_percent_full(sem_t* lock);

// Set the most recently picked item
void set_picked(sem_t* lock, int input, double perc);

// Callback for scale theads to handle the pick
void* picked(void *arg);

#endif
