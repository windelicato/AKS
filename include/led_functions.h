#ifndef LED_FUNCTIONS_H
#define LED_FUNCTIONS_H

int led_init();

void load_led_setup(int* led_ICs, int* led_Pins);

void remap_led_numbers(int* new_numbers);

int setLED(int LED, char PWM_value);

int led_shutdown();

#endif
