/*
 * EtchASketch.c
 *
 * AVR128DB48 + SH1106 application runner
 */

#define F_CPU 16000000UL

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stddef.h>

#include "Apps/etch_sketch.h"
#include "Drivers/ADC.h"
#include "Drivers/I2C.h"
#include "Drivers/SH1106.h"
#include "Drivers/TCA.h"
#include "Drivers/uart.h"

typedef enum
{
	APP_ETCH_SKETCH = 0
} AppId_t;

static void System_Init(void);
static void Clock_Init(void);
static void App_Init(AppId_t app);
static void App_Update(AppId_t app);

int main(void)
{
	AppId_t current_app = APP_ETCH_SKETCH;

	System_Init();
	App_Init(current_app);

	while (1)
	{
		App_Update(current_app);
	}
}

static void System_Init(void)
{
	Clock_Init();

	uart_init(3, 9600, NULL);

	ADC_Init();
	I2C_Init();
	SH1106_Init();
	TCA0_Init_1ms();

	sei();
}

static void Clock_Init(void)
{
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.XOSCHFCTRLA = CLKCTRL_FRQRANGE_16M_gc | CLKCTRL_ENABLE_bm;

	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_EXTCLK_gc;

	while (!(CLKCTRL.MCLKSTATUS & CLKCTRL_EXTS_bm));
}

static void App_Init(AppId_t app)
{
	switch (app)
	{
		case APP_ETCH_SKETCH:
			EtchSketch_Init();
			break;

		default:
			break;
	}
}

static void App_Update(AppId_t app)
{
	switch (app)
	{
		case APP_ETCH_SKETCH:
			EtchSketch_Update();
			break;

		default:
			break;
	}
}
