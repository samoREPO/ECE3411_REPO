/*
 * app.h
 *
 * Created: 4/15/2026 10:26:52 AM
 *  Author: samou
 */ 


#ifndef APP_H_
#define APP_H_

#include <stdint.h>

float adc_to_voltage(uint16_t sample);
void LCD_display_voltage_pair(float pot_voltage, float dpot_voltage);
void update_and_display_voltages(void);
void update_and_display_voltages_DPOT_DAC(void);

#endif /* APP_H_ */