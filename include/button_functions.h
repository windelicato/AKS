#ifndef BUTTON_FUNCTIONS_H
#define BUTTON_FUNCTIONS_H

int button_init();

void load_button_setup(int* button_ICs, int* button_Pins);

void remap_button_numbers(int* new_numbers);

int getButtonState(int button);

int getAllButtonsState(int* buffer);

int button_shutdown();

#endif
