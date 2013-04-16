#ifndef _SERIAL_H_
#define _SERIAL_H_
#include<semaphore.h>
#include<stdio.h>

struct scale{
	FILE* fid;
	int id;
	int lightbar;
	int sku;
	int quantity_needed;
	int hand_in_bin;
	double weight;
	double percent_full;
	sem_t *lock_weight;
	sem_t *lock_lightbar;
};

struct scale_list{
	int size;
	sem_t* sem_weight;
	sem_t* sem_lightbar;
	struct scale *scale;
};

void* picked(void *arg);

int check_weight_picked(int bin, sem_t* lock);

int check_lightbar_picked(int bin, sem_t* lock);

double check_percent_full(int bin, sem_t* lock);

int bins_init(struct scale_list* scales, int num_scales);

int scales_init(struct scale_list* scales);

#endif
