/*
 * I2C.h
 *
 * Created: 4/4/2026 11:54:50 AM
 *  Author: samou
 */ 


#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

#define I2C_WRITE 0
#define I2C_READ  1

void I2C_Init(void);
void I2C_Stop(void);

uint8_t I2C_Address(uint8_t address, uint8_t mode);
uint8_t I2C_WriteByte(uint8_t data);
uint8_t I2C_ReadByteNACK(void);

uint8_t I2C_Write(uint8_t address, uint8_t data);
uint8_t I2C_WriteBytes(uint8_t address, const uint8_t *data, uint16_t length);

#endif /* I2C_H_ */