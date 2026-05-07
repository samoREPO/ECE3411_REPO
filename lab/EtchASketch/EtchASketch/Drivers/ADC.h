/*
 * ADC.h
 *
 * Created: 4/20/2026 2:45:20 PM
 *  Author: samou
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>

void ADC_Init(void);
uint16_t ADC_ReadChannel(uint8_t muxpos);

#endif /* ADC_H_ */