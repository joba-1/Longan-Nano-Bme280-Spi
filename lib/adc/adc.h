#ifndef ADC_H
#define ADC_H

void adc_init( double offset );
void adc_start_measurement();
void adc_get_measurement( double *temperature, double *vref );

#endif