#ifndef LIGHTBAR_FUNCTIONS_H
#define LIGHTBAR_FUNCTIONS_H

//By: Matt Smith
//Lightbar Functions

void load_lightbar_setup(int* enable_ICs, int* enable_Pins, int* data_ICs, int* data_Pins);
<<<<<<< HEAD

void remap_lightbar_numbers(int* new_numbers);

=======

void remap_lightbar_numbers(int* new_numbers);

>>>>>>> cd8a344e1562cbe025da487986df2a37154e67b3
int enableLightBar(int lightbar);

int disableLightBar(int lightbar);

int readLightBar(int lightbar);

#endif
