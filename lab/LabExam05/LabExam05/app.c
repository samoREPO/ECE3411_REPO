/*
 * app.c
 *
 * Created: 4/15/2026
 *  Author: samou
 *
 * Application-level behavior for LabExam5.
 * This file coordinates peripherals without owning the low-level drivers.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app.h"
#include "ADC.h"
#include "DAC.h"
#include "lcd_lib.h"
#include "MPC4131.h"
#include "AnalogComparator.h"

#define ADC_REF_VOLTAGE 3.3f
#define ADC_COUNTS      4096.0f
#define DAC_COUNTS		1024.0f

static uint8_t config_MPC4131(void)
{
	uint8_t curr_setting = 0x00;

	for (int8_t i = 6; i >= 0; i--)
	{
		uint8_t guess = curr_setting | (uint8_t)(1U << i);
		MPC4131_write(guess);

		if (AC_is_output_high())
		{
			curr_setting |= (0x01 << i);
		}
	}

	return curr_setting;
}

static uint16_t config_DAC(uint16_t pot2_raw)
{
	uint16_t conversion = (((float)(pot2_raw * DAC_COUNTS)) / ADC_COUNTS) ;


	DAC_ouput(conversion);

	return conversion;
}

float adc_to_voltage(uint16_t sample)
{
	return ((float)sample / ADC_COUNTS) * ADC_REF_VOLTAGE;
}

void LCD_display_voltage_pair(float pot_voltage, float dpot_voltage)
{
	char VoltageBuffer[6];
	
	dtostrf(pot_voltage, 4, 2, VoltageBuffer);
	char LCDout [20] = "V DAC: ";
	strcat(LCDout, VoltageBuffer);
	
	LCDgotoXY(0, 0);
	LCDstring(LCDout);

	dtostrf(dpot_voltage, 4, 2, VoltageBuffer);
	char LCD2out [20] = "V DPOT: ";
	strcat(LCD2out, VoltageBuffer);
	
	LCDgotoXY(0, 1);
	LCDstring(LCD2out);
}

void update_and_display_voltages(void)
{
	uint16_t pot1_raw;
	uint16_t pot2_raw;
	uint16_t dpot_raw;
	uint16_t dac_raw;
	float dac_voltage;
	float dpot_voltage;

	pot1_raw = read_ADC1();
	pot2_raw = read_ADC3();
	
	
	config_DAC(pot2_raw);

	config_MPC4131();

	dpot_raw = read_ADC2();
	dac_raw = read_ADC4();

	dac_voltage = adc_to_voltage(dac_raw);
	dpot_voltage = adc_to_voltage(dpot_raw);

	LCD_display_voltage_pair(dac_voltage, dpot_voltage);
}

static uint8_t set_MPC4131_2Volts(void)
{
	uint8_t guess = 0b1001111;
	MPC4131_write(guess);

	return guess;
}

void update_and_display_voltages_DPOT_DAC(void)
{
	uint16_t pot_raw;
	uint16_t dpot_raw;
	float pot_voltage;
	float dpot_voltage;

	pot_raw = read_ADC1();

	set_MPC4131_2Volts();

	dpot_raw = read_ADC2();

	pot_voltage = adc_to_voltage(pot_raw);
	dpot_voltage = adc_to_voltage(dpot_raw);

	LCD_display_voltage_pair(pot_voltage, dpot_voltage);
}