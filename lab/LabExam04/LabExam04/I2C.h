/*
 * I2C.h
 *
 * Created: 4/6/2026 1:38:50 PM
 *  Author: samou
 */ 


#include <stdbool.h>

void TWI_Host_Initialize(bool enable_pullups);
void TWI_Host_Write(uint8_t Address, uint8_t Data);
void TWI_Host_Write_16bits(uint8_t Address, uint16_t Data);
void TWI_Host_Write_Bytes(uint8_t Address, uint8_t* Data, int numbytes);
uint8_t TWI_Host_Read(uint8_t Address);
void TWI_Address(uint8_t Address, uint8_t mode);
void TWI_Stop();
int TWI_Transmit_Data(uint8_t data);
uint8_t TWI_Receive_Data();