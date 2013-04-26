#ifndef _SERIAL_H_
#define _SERIAL_H_
#include<semaphore.h>

struct scale{
	FILE* fid;
	int id;
	int lightbar;
	int led;
	int button;
	int sku;
	int mode;
	int quantity_needed;
	int hand_in_bin;
	int use_given_weight;
	double weight;
	double percent_full;
	sem_t *lock_weight;
	sem_t *lock_lightbar;
	sem_t *lock_restock;
};

struct scale_list{
	int num_scales;
	int num_pseudo_scales;
	sem_t* sem_weight;
	sem_t* sem_lightbar;
	sem_t* sem_restock;
	struct scale *scale;
};

void* picked(void *arg);
void* restock_indicator(void *arg);

void check_scale_pick_data(int bin, sem_t* lock, int* state, int* num_objs, double* percent);

int check_weight_picked(int bin, sem_t* lock);

int check_lightbar_picked(int bin, sem_t* lock);

double check_percent_full(int bin, sem_t* lock);

int bins_init(struct scale_list* scales, int num_scales, int num_pseudo_scales);

int scales_init(struct scale_list* scales);

#endif
