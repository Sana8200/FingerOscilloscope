#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// 3-bit color
typedef enum {
    COLOR_BLACK   = 0b000,
    COLOR_BLUE    = 0b001,
    COLOR_GREEN   = 0b010,
    COLOR_CYAN    = 0b011,
    COLOR_RED     = 0b100,
    COLOR_MAGENTA = 0b101,
    COLOR_YELLOW  = 0b110,
    COLOR_WHITE   = 0b111
} Color;

// Framebuffer
extern Color framebuffer[SCREEN_HEIGHT][SCREEN_WIDTH];

// Function prototypes
void set_pixel(int x, int y, Color color);
void clear_screen(Color color);
void draw_waveform(void);

#endif
