//By: Matt Smith
//Lightbar Functions

void load_lightbar_setup(int* enable_ICs, int* enable_Pins, int* data_ICs, int* data_Pins);

void remap_lightbar_numbers(int* new_numbers);

int enableLightBar(int lightbar);

int disableLightBar(int lightbar);

int readLightBar(int lightbar);
