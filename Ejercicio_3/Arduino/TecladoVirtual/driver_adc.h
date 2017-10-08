#ifndef DRIVER_ADC
#define DRIVER_ADC

typedef struct
{
	int canal;
	void (*callback)();
  int analogValue;
} miestruct;

int adc_init(miestruct *cfg);

void adc_loop();

#endif
