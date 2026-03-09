/*
 * I2C_lib.h
 *
 * Created: 3/8/2026 11:00:40 AM
 *  Author: samou
 */ 


#ifndef I2C_LIB_H_
#define I2C_LIB_H_

#include <inttypes.h>

// I2C prototypes
void TWI_Host_Initialize(void);
void TWI_Stop(void);
void TWI_Address(uint8_t , uint8_t);
int TWI_Transmit_Data(uint8_t);
uint8_t TWI_Receive_Data(void);

#endif /* I2C_LIB_H_ */