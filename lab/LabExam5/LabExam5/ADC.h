/*
 * ADC.h
 *
 * Created: 4/15/2026 9:11:14 AM
 *  Author: samou
 */ 

#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>

void init_ADC(void);
uint16_t read_ADC1(void);
uint16_t read_ADC2(void);



#endif /* ADC_H_ */