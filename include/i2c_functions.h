//By: Matt Smith
//I2C Functions

//Sets up the GPIO expander ICs
void setup_i2c_GPIO();

//Sets the pin on the given IC to the given (0 or 1) value
//ICs are ordered by their i2c addresses from lowest to highest and assigned numbers 0 - (number of ICs)
void i2c_setGPIOPin(int chip, int pin, char value);

void i2c_setGPIOPins(int chip, char* pins);

//Reads the pin on the given IC and returns a 0 for a low and a 1 for a high
//ICs are ordered by their i2c addresses from lowest to highest and assigned numbers 0 - (number of ICs)
int i2c_readGPIOPin(int chip, int pin);

void i2c_writePins(int chip);

