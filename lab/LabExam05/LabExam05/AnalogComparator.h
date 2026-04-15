/*
 * AnalogComparator.h
 *
 * Created: 4/15/2026 9:25:44 AM
 *  Author: samou
 */ 


#ifndef ANALOGCOMPARATOR_H_
#define ANALOGCOMPARATOR_H_

#include <stdbool.h>

void init_AC(void);
bool AC_is_output_high(void);
void AC_DPOT(void);
void AC_DAC(void);

#endif /* ANALOGCOMPARATOR_H_ */