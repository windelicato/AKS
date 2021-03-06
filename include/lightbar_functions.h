#ifndef LIGHTBAR_FUNCTIONS_H
#define LIGHTBAR_FUNCTIONS_H

//By: Matt Smith
//Lightbar Functions

int lightbar_init();

void load_lightbar_setup(int* enable_ICs, int* enable_Pins, int* data_ICs, int* data_Pins);

void remap_lightbar_numbers(int* new_numbers);

int enableLightBar(int lightbar);

int disableLightBar(int lightbar);

int readLightBar(int lightbar);

int lightbar_shutdown();

#endif
