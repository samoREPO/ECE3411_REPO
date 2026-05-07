/*
 * ADC.h
 *
 * Created: 4/15/2026 9:11:14 AM
 *  Author: samou
 */ 

#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>
#include <avr/io.h>

void init_ADC(void);

/* Generic helper */
uint16_t adc_read_channel(uint8_t muxpos);

/* Fixed channel mappings */
uint16_t read_ADC1(void);   // POT1
uint16_t read_ADC2(void);   // DPOT feedback
uint16_t read_ADC3(void);   // POT2
uint16_t read_ADC4(void);   // DAC feedback

#endif /* ADC_H_ */