/*
 * gfx.c
 *
 * Created: 4/26/2026 8:06:13 AM
 *  Author: samou
 */ 

#include "gfx.h"
#include "../Drivers/SH1106.h"

static int16_t abs_i16(int16_t value)
{
	return (value < 0) ? -value : value;
}

static uint8_t is_visible(int16_t x, int16_t y)
{
	return (x >= 0 && x < GFX_WIDTH && y >= 0 && y < GFX_HEIGHT);
}

void GFX_Clear(void)
{
	SH1106_ClearBuffer();
}

void GFX_Fill(void)
{
	SH1106_FillBuffer();
}

void GFX_Update(void)
{
	SH1106_UpdateScreen();
}

void GFX_InvertDisplay(uint8_t enable)
{
	SH1106_InvertDisplay(enable);
}

void GFX_DrawPixel(int16_t x, int16_t y, GFX_Color_t color)
{
	if (!is_visible(x, y))
	{
		return;
	}

	SH1106_WritePixel((uint8_t)x, (uint8_t)y, color == GFX_COLOR_ON);
}

void GFX_DrawHLine(int16_t x, int16_t y, int16_t width, GFX_Color_t color)
{
	if (width < 0)
	{
		x += width + 1;
		width = -width;
	}

	for (int16_t i = 0; i < width; i++)
	{
		GFX_DrawPixel(x + i, y, color);
	}
}

void GFX_DrawVLine(int16_t x, int16_t y, int16_t height, GFX_Color_t color)
{
	if (height < 0)
	{
		y += height + 1;
		height = -height;
	}

	for (int16_t i = 0; i < height; i++)
	{
		GFX_DrawPixel(x, y + i, color);
	}
}

void GFX_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, GFX_Color_t color)
{
	int16_t dx = abs_i16(x1 - x0);
	int16_t sx = (x0 < x1) ? 1 : -1;
	int16_t dy = -abs_i16(y1 - y0);
	int16_t sy = (y0 < y1) ? 1 : -1;
	int16_t err = dx + dy;

	while (1)
	{
		GFX_DrawPixel(x0, y0, color);

		if (x0 == x1 && y0 == y1)
		{
			break;
		}

		int16_t e2 = 2 * err;

		if (e2 >= dy)
		{
			err += dy;
			x0 += sx;
		}

		if (e2 <= dx)
		{
			err += dx;
			y0 += sy;
		}
	}
}

void GFX_DrawDottedLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, GFX_Color_t color)
{
	int16_t dx = abs_i16(x1 - x0);
	int16_t sx = (x0 < x1) ? 1 : -1;
	int16_t dy = -abs_i16(y1 - y0);
	int16_t sy = (y0 < y1) ? 1 : -1;
	int16_t err = dx + dy;
	uint8_t step = 0;

	while (1)
	{
		if ((step % 2) == 0)
		{
			GFX_DrawPixel(x0, y0, color);
		}

		if (x0 == x1 && y0 == y1)
		{
			break;
		}

		int16_t e2 = 2 * err;

		if (e2 >= dy)
		{
			err += dy;
			x0 += sx;
		}

		if (e2 <= dx)
		{
			err += dx;
			y0 += sy;
		}

		step++;
	}
}

void GFX_DrawRect(int16_t x, int16_t y, int16_t width, int16_t height, GFX_Color_t color)
{
	if (width == 0 || height == 0)
	{
		return;
	}

	if (width < 0)
	{
		x += width + 1;
		width = -width;
	}

	if (height < 0)
	{
		y += height + 1;
		height = -height;
	}

	GFX_DrawHLine(x, y, width, color);
	GFX_DrawHLine(x, y + height - 1, width, color);
	GFX_DrawVLine(x, y, height, color);
	GFX_DrawVLine(x + width - 1, y, height, color);
}

void GFX_FillRect(int16_t x, int16_t y, int16_t width, int16_t height, GFX_Color_t color)
{
	if (width < 0)
	{
		x += width + 1;
		width = -width;
	}

	if (height < 0)
	{
		y += height + 1;
		height = -height;
	}

	for (int16_t i = 0; i < height; i++)
	{
		GFX_DrawHLine(x, y + i, width, color);
	}
}

void GFX_DrawCircle(int16_t x0, int16_t y0, int16_t radius, GFX_Color_t color)
{
	if (radius < 0)
	{
		return;
	}

	int16_t x = radius;
	int16_t y = 0;
	int16_t err = 0;

	while (x >= y)
	{
		GFX_DrawPixel(x0 + x, y0 + y, color);
		GFX_DrawPixel(x0 + y, y0 + x, color);
		GFX_DrawPixel(x0 - y, y0 + x, color);
		GFX_DrawPixel(x0 - x, y0 + y, color);
		GFX_DrawPixel(x0 - x, y0 - y, color);
		GFX_DrawPixel(x0 - y, y0 - x, color);
		GFX_DrawPixel(x0 + y, y0 - x, color);
		GFX_DrawPixel(x0 + x, y0 - y, color);

		y++;

		if (err <= 0)
		{
			err += (2 * y) + 1;
		}

		if (err > 0)
		{
			x--;
			err -= (2 * x) + 1;
		}
	}
}

void GFX_FillCircle(int16_t x0, int16_t y0, int16_t radius, GFX_Color_t color)
{
	if (radius < 0)
	{
		return;
	}

	int16_t x = radius;
	int16_t y = 0;
	int16_t err = 0;

	while (x >= y)
	{
		GFX_DrawHLine(x0 - x, y0 + y, (2 * x) + 1, color);
		GFX_DrawHLine(x0 - x, y0 - y, (2 * x) + 1, color);
		GFX_DrawHLine(x0 - y, y0 + x, (2 * y) + 1, color);
		GFX_DrawHLine(x0 - y, y0 - x, (2 * y) + 1, color);

		y++;

		if (err <= 0)
		{
			err += (2 * y) + 1;
		}

		if (err > 0)
		{
			x--;
			err -= (2 * x) + 1;
		}
	}
}
