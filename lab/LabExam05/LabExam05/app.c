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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "app.h"
#include "ADC.h"
#include "DAC.h"
#include "lcd_lib.h"
#include "MPC4131.h"

#define VREF_ADC        3.3f
#define ADC_MAX_COUNTS  4095.0f
#define DAC_MAX_COUNTS  1023.0f
#define DPOT_MAX_COUNTS 127.0f

static uint8_t pot1_to_dpot_code(uint16_t pot1_raw)
{
    return (uint8_t)((pot1_raw * DPOT_MAX_COUNTS) / ADC_MAX_COUNTS);
}

static uint16_t pot2_to_dac_code(uint16_t pot2_raw)
{
    return (uint16_t)((pot2_raw * DAC_MAX_COUNTS) / ADC_MAX_COUNTS);
}

float adc_to_voltage(uint16_t sample)
{
    return ((float)sample * VREF_ADC) / ADC_MAX_COUNTS;
}

void display_two_voltages(float dac_v, float dpot_v)
{
    char line[17];
    char num[8];

    LCDclr();

    dtostrf(dac_v, 4, 2, num);
    snprintf(line, sizeof(line), "V DAC: %s", num);
    LCDgotoXY(0, 0);
    LCDstring(line);

    dtostrf(dpot_v, 4, 2, num);
    snprintf(line, sizeof(line), "V DPOT:%s", num);
    LCDgotoXY(0, 1);
    LCDstring(line);
}

void update_outputs_and_display(void)
{
    uint16_t pot1_raw = read_ADC1();
    uint16_t pot2_raw = read_ADC3();

    uint8_t dpot_code = pot1_to_dpot_code(pot1_raw);
    uint16_t dac_code = pot2_to_dac_code(pot2_raw);

    MPC4131_write(dpot_code);
    DAC_ouput(dac_code);

    uint16_t dpot_meas_raw = read_ADC2();
    uint16_t dac_meas_raw  = read_ADC4();

    float dpot_v = adc_to_voltage(dpot_meas_raw);
    float dac_v  = adc_to_voltage(dac_meas_raw);

    display_two_voltages(dac_v, dpot_v);
}

float measure_difference(void)
{
    uint16_t dpot_meas_raw = read_ADC2();
    uint16_t dac_meas_raw  = read_ADC4();

    float dpot_v = adc_to_voltage(dpot_meas_raw);
    float dac_v  = adc_to_voltage(dac_meas_raw);

    if (dac_v >= dpot_v) {
        return dac_v - dpot_v;
    } else {
        return dpot_v - dac_v;
    }
}

void display_difference(float diff)
{
    char line[17];
    char num[8];

    LCDclr();

    dtostrf(diff, 4, 2, num);
    snprintf(line, sizeof(line), "V DIFF:%s", num);
    LCDgotoXY(0, 0);
    LCDstring(line);
}