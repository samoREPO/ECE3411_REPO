/*
 * app.h
 *
 * Created: 4/15/2026 10:26:52 AM
 *  Author: samou
 */ 


#ifndef APP_H_
#define APP_H_

#include <stdint.h>

/* Conversion helpers */
float adc_to_voltage(uint16_t sample);

/* Main application behavior */
void update_outputs_and_display(void);
float measure_difference(void);

/* LCD helpers */
void display_two_voltages(float dac_v, float dpot_v);
void display_difference(float diff);

#endif /* APP_H_ */