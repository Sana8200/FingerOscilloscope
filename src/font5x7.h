#include <stdint.h>
#include "vga_driver.h"

static const uint8_t font_5x7[][5];
int get_char_idx(char c);
void vga_draw_string(int x, int y, char *str, uint8_t color);
void vga_draw_int(int x, int y, int val, uint16_t color);
static void draw_float(int x, int y, float val, int dec, uint16_t color);