/*
 * SPI.h
 *
 * Created: 4/15/2026 9:20:04 AM
 *  Author: samou
 */ 

#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>

void SPI_Host_Init(void);
uint8_t SPI_Transfer(uint8_t data);
void SPI_Select(void);
void SPI_Deselect(void);

void SPI_Client_Init(void);
uint8_t SPI_Client_Receive(void);

#endif /* SPI_H_ */