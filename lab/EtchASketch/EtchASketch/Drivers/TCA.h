/*
 * TCA.h
 *
 * Created: 4/25/2026 9:32:30 AM
 *  Author: samou
 */ 


#ifndef TCA_H_
#define TCA_H_

#include <stdint.h>

void TCA0_Init_1ms(void);

uint32_t TCA0_Millis(void);
uint8_t TCA0_HasElapsed(uint32_t *previous_time, uint32_t interval_ms);

#endif /* TCA_H_ */