#ifndef GFX_H_
#define GFX_H_

#include <stdint.h>

void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1);

void gfx_cursor_hide(void);
void gfx_cursor_blink(uint8_t x, uint8_t y, uint16_t blink_period_ticks);

void draw_char(uint8_t x, uint8_t y, char c);
void draw_string(uint8_t x, uint8_t y, const char *str);

#endif /* GFX_H_ */