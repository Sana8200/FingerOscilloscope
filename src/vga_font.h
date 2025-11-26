#ifndef VGA_FONT
#define VGA_FONT

#include "vga_driver.h"
#include <stdint.h>

static const uint8_t font[91][5];

void vga_draw_char(int x, int y, char c, uint16_t color);
void vga_draw_string(int x, int y, const char *s, uint16_t color);
void vga_draw_int(int x, int y, int val, uint16_t color);
void draw_float(int x, int y, float val, int dec, uint16_t color);

#endif  //VGA_FONT