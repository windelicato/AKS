#ifndef _SERIAL_H_
#define _SERIAL_H_
#include<semaphore.h>

struct scale{
	FILE* fid;
	int id;
	int sku;
	double weight;
	sem_t *lock;
};

struct scale_list{
	int size;
	sem_t sem;
	struct scale *scale;
};

int check_picked(sem_t* lock);

void set_picked(sem_t* lock, int inpu);

void* picked(void *arg);

#endif
