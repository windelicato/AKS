//By: Matt Smith
//I2C Functions

<<<<<<< HEAD
=======
const int i2c_gpio_ic_pin_bytes, i2c_led_ic_pwm_bytes;

>>>>>>> cd8a344e1562cbe025da487986df2a37154e67b3
//Initializes i2c devices
int i2c_init();

//Sets up the GPIO expander ICs
int setup_i2c_GPIO();

//Sets up the LED Driver ICs
int setup_i2c_LED_Drivers();

//Functions used internally to select the correct IC on the I2C bus
int select_GPIO_IC(int chip);
int select_LED_Driver_IC(int chip);

//Sets the pin on the given IC to the given (0 or 1) value
//ICs are ordered by their i2c addresses from lowest to highest and assigned numbers 0 - (number of ICs)
int i2c_setGPIOPin(int chip, int pin, char value);

int i2c_setLEDDriverPin(int chip, int pin, char value);

int i2c_setGPIOPins(int chip, char* pins);

int i2c_setLEDDriverPins(int chip, char* pins);

//Reads the pin on the given IC and returns a 0 for a low and a 1 for a high
//ICs are ordered by their i2c addresses from lowest to highest and assigned numbers 0 - (number of ICs)
int i2c_readGPIOPin(int chip, int pin);

int i2c_readLEDDriverPin(int chip, int pin);

int i2c_readGPIOPins(int chip, char* buf);

int i2c_readLEDDriverPins(int chip, char* buf);

int i2c_writeGPIOPins(int chip);

void i2c_functions_dealloc();
