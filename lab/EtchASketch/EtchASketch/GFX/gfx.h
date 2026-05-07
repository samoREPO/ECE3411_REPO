/*
 * gfx.h
 *
 * Created: 4/26/2026 8:06:24 AM
 *  Author: samou
 */ 


#ifndef GFX_H_
#define GFX_H_

#include <stdint.h>

#define GFX_WIDTH   128
#define GFX_HEIGHT  64

typedef enum
{
	GFX_COLOR_OFF = 0,
	GFX_COLOR_ON = 1
} GFX_Color_t;

void GFX_Clear(void);
void GFX_Fill(void);
void GFX_Update(void);
void GFX_InvertDisplay(uint8_t enable);

void GFX_DrawPixel(int16_t x, int16_t y, GFX_Color_t color);
void GFX_DrawHLine(int16_t x, int16_t y, int16_t width, GFX_Color_t color);
void GFX_DrawVLine(int16_t x, int16_t y, int16_t height, GFX_Color_t color);
void GFX_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, GFX_Color_t color);
void GFX_DrawDottedLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, GFX_Color_t color);

void GFX_DrawRect(int16_t x, int16_t y, int16_t width, int16_t height, GFX_Color_t color);
void GFX_FillRect(int16_t x, int16_t y, int16_t width, int16_t height, GFX_Color_t color);

void GFX_DrawCircle(int16_t x0, int16_t y0, int16_t radius, GFX_Color_t color);
void GFX_FillCircle(int16_t x0, int16_t y0, int16_t radius, GFX_Color_t color);

#endif /* GFX_H_ */
